// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

extern crate proc_macro;
extern crate proc_macro2;
#[macro_use]
extern crate quote;
extern crate syn;

use proc_macro::TokenStream;
use proc_macro2::{Span, TokenStream as TokenStream2};
use syn::*;

fn expr_as_usize(expr: &syn::Expr) -> usize {
    let lit = match expr {
        syn::Expr::Lit(lit) => lit,
        _ => panic!("Expected a literal, found an expression"),
    };
    let lit_int = match &lit.lit {
        syn::Lit::Int(i) => i,
        _ => panic!("Expected a literal integer"),
    };
    assert!(lit.attrs.is_empty());
    lit_int
        .base10_parse()
        .expect("Failed to parse integer literal")
}

fn count_type(ty: &Type, search_type: &str) -> usize {
    match ty {
        syn::Type::Array(a) => {
            let elems = count_type(a.elem.as_ref(), search_type);
            if elems > 0 {
                elems * expr_as_usize(&a.len)
            } else {
                0
            }
        }
        syn::Type::Path(p) => {
            if p.qself.is_none() && p.path.is_ident(search_type) {
                1
            } else {
                0
            }
        }
        _ => 0,
    }
}

fn get_src_type(field: &Field) -> Option<String> {
    for attr in &field.attrs {
        if let Meta::List(ml) = &attr.meta {
            if ml.path.is_ident("src_type") {
                return Some(format!("{}", ml.tokens));
            }
        }
    }
    None
}

fn derive_as_slice(
    input: TokenStream,
    trait_name: &str,
    func_prefix: &str,
    search_type: &str,
) -> TokenStream {
    let DeriveInput {
        attrs, ident, data, ..
    } = parse_macro_input!(input);

    let trait_name = Ident::new(trait_name, Span::call_site());
    let elem_type = Ident::new(search_type, Span::call_site());
    let as_slice =
        Ident::new(&format!("{}_as_slice", func_prefix), Span::call_site());
    let as_mut_slice =
        Ident::new(&format!("{}_as_mut_slice", func_prefix), Span::call_site());

    match data {
        Data::Struct(s) => {
            let mut has_repr_c = false;
            for attr in attrs {
                match attr.meta {
                    Meta::List(ml) => {
                        if ml.path.is_ident("repr")
                            && format!("{}", ml.tokens) == "C"
                        {
                            has_repr_c = true;
                        }
                    }
                    _ => (),
                }
            }
            assert!(has_repr_c, "Struct must be declared #[repr(C)]");

            let mut first = None;
            let mut count = 0_usize;
            let mut found_last = false;
            let mut src_types = TokenStream2::new();

            if let Fields::Named(named) = s.fields {
                for f in named.named {
                    let ty_count = count_type(&f.ty, search_type);

                    if search_type == "Src" {
                        let src_type = get_src_type(&f);
                        if ty_count == 0 && !src_type.is_none() {
                            panic!(
                                "src_type attribute is only allowed on sources"
                            );
                        }

                        let src_type = if let Some(s) = src_type {
                            let s = syn::parse_str::<Ident>(&s).unwrap();
                            quote! { SrcType::#s, }
                        } else {
                            quote! { SrcType::DEFAULT, }
                        };

                        for _ in 0..ty_count {
                            src_types.extend(src_type.clone());
                        }
                    }

                    if ty_count > 0 {
                        assert!(
                            !found_last,
                            "All fields of type {} must be consecutive",
                            search_type
                        );
                        first.get_or_insert(f.ident);
                        count += ty_count;
                    } else {
                        if !first.is_none() {
                            found_last = true;
                        }
                    }
                }
            } else {
                panic!("Fields are not named");
            }

            let src_type_func = if search_type == "Src" {
                quote! {
                    fn src_types(&self) -> SrcTypeList {
                        static SRC_TYPES: [SrcType; #count]  = [#src_types];
                        SrcTypeList::Array(&SRC_TYPES)
                    }
                }
            } else {
                TokenStream2::new()
            };

            if let Some(name) = first {
                quote! {
                    impl #trait_name for #ident {
                        fn #as_slice(&self) -> &[#elem_type] {
                            unsafe {
                                let first = &self.#name as *const #elem_type;
                                std::slice::from_raw_parts(first, #count)
                            }
                        }

                        fn #as_mut_slice(&mut self) -> &mut [#elem_type] {
                            unsafe {
                                let first = &mut self.#name as *mut #elem_type;
                                std::slice::from_raw_parts_mut(first, #count)
                            }
                        }

                        #src_type_func
                    }
                }
            } else {
                quote! {
                    impl #trait_name for #ident {
                        fn #as_slice(&self) -> &[#elem_type] {
                            &[]
                        }

                        fn #as_mut_slice(&mut self) -> &mut [#elem_type] {
                            &mut []
                        }

                        #src_type_func
                    }
                }
            }
            .into()
        }
        Data::Enum(e) => {
            let mut as_slice_cases = TokenStream2::new();
            let mut as_mut_slice_cases = TokenStream2::new();
            let mut src_types_cases = TokenStream2::new();
            for v in e.variants {
                let case = v.ident;
                as_slice_cases.extend(quote! {
                    #ident::#case(x) => x.#as_slice(),
                });
                as_mut_slice_cases.extend(quote! {
                    #ident::#case(x) => x.#as_mut_slice(),
                });
                if search_type == "Src" {
                    src_types_cases.extend(quote! {
                        #ident::#case(x) => x.src_types(),
                    });
                }
            }
            let src_type_func = if search_type == "Src" {
                quote! {
                    fn src_types(&self) -> SrcTypeList {
                        match self {
                            #src_types_cases
                        }
                    }
                }
            } else {
                TokenStream2::new()
            };
            quote! {
                impl #trait_name for #ident {
                    fn #as_slice(&self) -> &[#elem_type] {
                        match self {
                            #as_slice_cases
                        }
                    }

                    fn #as_mut_slice(&mut self) -> &mut [#elem_type] {
                        match self {
                            #as_mut_slice_cases
                        }
                    }

                    #src_type_func
                }
            }
            .into()
        }
        _ => panic!("Not a struct type"),
    }
}

#[proc_macro_derive(SrcsAsSlice, attributes(src_type))]
pub fn derive_srcs_as_slice(input: TokenStream) -> TokenStream {
    derive_as_slice(input, "SrcsAsSlice", "srcs", "Src")
}

#[proc_macro_derive(DstsAsSlice)]
pub fn derive_dsts_as_slice(input: TokenStream) -> TokenStream {
    derive_as_slice(input, "DstsAsSlice", "dsts", "Dst")
}

#[proc_macro_derive(DisplayOp)]
pub fn enum_derive_display_op(input: TokenStream) -> TokenStream {
    let DeriveInput { ident, data, .. } = parse_macro_input!(input);

    if let Data::Enum(e) = data {
        let mut fmt_dsts_cases = TokenStream2::new();
        let mut fmt_op_cases = TokenStream2::new();
        for v in e.variants {
            let case = v.ident;
            fmt_dsts_cases.extend(quote! {
                #ident::#case(x) => x.fmt_dsts(f),
            });
            fmt_op_cases.extend(quote! {
                #ident::#case(x) => x.fmt_op(f),
            });
        }
        quote! {
            impl DisplayOp for #ident {
                fn fmt_dsts(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                    match self {
                        #fmt_dsts_cases
                    }
                }

                fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                    match self {
                        #fmt_op_cases
                    }
                }
            }
        }
        .into()
    } else {
        panic!("Not an enum type");
    }
}

#[proc_macro_derive(FromVariants)]
pub fn derive_from_variants(input: TokenStream) -> TokenStream {
    let DeriveInput { ident, data, .. } = parse_macro_input!(input);
    let enum_type = ident;

    let mut impls = TokenStream2::new();

    if let Data::Enum(e) = data {
        for v in e.variants {
            let var_ident = v.ident;
            let from_type = match v.fields {
                Fields::Unnamed(FieldsUnnamed { unnamed, .. }) => unnamed,
                _ => panic!("Expected Op(OpFoo)"),
            };

            let quote = quote! {
                impl From<#from_type> for #enum_type {
                    fn from (op: #from_type) -> #enum_type {
                        #enum_type::#var_ident(op)
                    }
                }
            };

            impls.extend(quote);
        }
    }

    impls.into()
}
