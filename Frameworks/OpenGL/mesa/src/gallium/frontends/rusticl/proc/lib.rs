extern crate proc_macro;
use proc_macro::Delimiter;
use proc_macro::TokenStream;
use proc_macro::TokenTree::Group;
use proc_macro::TokenTree::Ident;
use proc_macro::TokenTree::Punct;

/// Macro for generating the C API stubs for normal functions
#[proc_macro_attribute]
pub fn cl_entrypoint(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let mut name = None;
    let mut args = None;
    let mut ret_type = None;

    let mut iter = item.clone().into_iter();
    while let Some(item) = iter.next() {
        match item {
            Ident(ident) => match ident.to_string().as_str() {
                // extract the function name
                "fn" => name = Some(iter.next().unwrap().to_string()),

                // extract inner type
                "CLResult" => {
                    // skip the `<`
                    iter.next();
                    let mut ret_type_tmp = String::new();

                    for ident in iter.by_ref() {
                        if ident.to_string() == ">" {
                            break;
                        }

                        if ret_type_tmp.ends_with("mut") || ret_type_tmp.ends_with("const") {
                            ret_type_tmp.push(' ');
                        }

                        ret_type_tmp.push_str(ident.to_string().as_str());
                    }

                    ret_type = Some(ret_type_tmp);
                }
                _ => {}
            },
            Group(group) => {
                if args.is_some() {
                    continue;
                }

                if group.delimiter() != Delimiter::Parenthesis {
                    continue;
                }

                // the first group are our function args :)
                args = Some(group.stream());
            }
            _ => {}
        }
    }

    let name = name.as_ref().expect("no name found!");
    let args = args.as_ref().expect("no args found!");
    let ret_type = ret_type.as_ref().expect("no ret_type found!");

    let mut arg_names = Vec::new();
    let mut collect = true;

    // extract the variable names of our function arguments
    for item in args.clone() {
        match item {
            Ident(ident) => {
                if collect {
                    arg_names.push(ident);
                }
            }

            // we ignore everything between a `:` and a `,` as those are the argument types
            Punct(punct) => match punct.as_char() {
                ':' => collect = false,
                ',' => collect = true,
                _ => {}
            },

            _ => {}
        }
    }

    // convert to string and strip `mut` specifiers
    let arg_names: Vec<_> = arg_names
        .clone()
        .into_iter()
        .map(|ident| ident.to_string())
        .filter(|ident| ident != "mut")
        .collect();

    let arg_names_str = arg_names.join(",");
    let mut args = args.to_string();
    if !args.ends_with(',') {
        args.push(',');
    }

    // depending on the return type we have to generate a different match case
    let mut res: TokenStream = if ret_type == "()" {
        // trivial case: return the `Err(err)` as is
        format!(
            "pub extern \"C\" fn cl_{name}(
                {args}
            ) -> cl_int {{
                match {name}({arg_names_str}) {{
                    Ok(_) => CL_SUCCESS as cl_int,
                    Err(e) => e,
                }}
            }}"
        )
    } else {
        // here we write the error code into the last argument, which we also add. All OpenCL APIs
        // which return an object do have the `errcode_ret: *mut cl_int` argument last, so we can
        // just make use of this here.
        format!(
            "pub extern \"C\" fn cl_{name}(
                {args}
                errcode_ret: *mut cl_int,
            ) -> {ret_type} {{
                let (ptr, err) = match {name}({arg_names_str}) {{
                    Ok(o) => (o, CL_SUCCESS as cl_int),
                    Err(e) => (std::ptr::null_mut(), e),
                }};
                if !errcode_ret.is_null() {{
                    unsafe {{
                        *errcode_ret = err;
                    }}
                }}
                ptr
            }}"
        )
    }
    .parse()
    .unwrap();

    res.extend(item);
    res
}

/// Special macro for generating C function stubs to call into our `CLInfo` trait
#[proc_macro_attribute]
pub fn cl_info_entrypoint(attr: TokenStream, item: TokenStream) -> TokenStream {
    let mut name = None;
    let mut args = Vec::new();
    let mut iter = item.clone().into_iter();

    let mut collect = false;

    // we have to extract the type name we implement the trait for and the type of the input
    // parameters. The input Parameters are defined as `T` inside `CLInfo<T>` or `CLInfoObj<T, ..>`
    while let Some(item) = iter.next() {
        match item {
            Ident(ident) => {
                if collect {
                    args.push(ident);
                } else if ident.to_string() == "for" {
                    name = Some(iter.next().unwrap().to_string());
                }
            }
            Punct(punct) => match punct.as_char() {
                '<' => collect = true,
                '>' => collect = false,
                _ => {}
            },
            _ => {}
        }
    }

    let name = name.as_ref().expect("no name found!");
    assert!(!args.is_empty());

    // the 1st argument is special as it's the actual property being queried. The remaining
    // arguments are additional input data being passed before the property.
    let arg = &args[0];
    let (args_values, args) = args[1..]
        .iter()
        .enumerate()
        .map(|(idx, arg)| (format!("arg{idx},"), format!("arg{idx}: {arg},")))
        .reduce(|(a1, b1), (a2, b2)| (a1 + &a2, b1 + &b2))
        .unwrap_or_default();

    // depending on the amount of arguments we have a different trait implementation
    let method = if args.len() > 1 {
        "get_info_obj"
    } else {
        "get_info"
    };

    let mut res: TokenStream = format!(
        "pub extern \"C\" fn {attr}(
            input: {name},
            {args}
            param_name: {arg},
            param_value_size: usize,
            param_value: *mut ::std::ffi::c_void,
            param_value_size_ret: *mut usize,
        ) -> cl_int {{
            match input.{method}(
                {args_values}
                param_name,
                param_value_size,
                param_value,
                param_value_size_ret,
            ) {{
                Ok(_) => CL_SUCCESS as cl_int,
                Err(e) => e,
            }}
        }}"
    )
    .parse()
    .unwrap();

    res.extend(item);
    res
}
