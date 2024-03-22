pub struct Properties<T> {
    pub props: Vec<(T, T)>,
}

impl<T: Copy + PartialEq + Default> Properties<T> {
    #[allow(clippy::not_unsafe_ptr_arg_deref)]
    pub fn from_ptr_raw(mut p: *const T) -> Vec<T> {
        let mut res: Vec<T> = Vec::new();

        if !p.is_null() {
            unsafe {
                while *p != T::default() {
                    res.push(*p);
                    res.push(*p.add(1));
                    p = p.add(2);
                }
            }
            res.push(T::default());
        }

        res
    }

    #[allow(clippy::not_unsafe_ptr_arg_deref)]
    pub fn from_ptr(mut p: *const T) -> Option<Self> {
        let mut res = Self::default();

        if !p.is_null() {
            let mut k: Vec<T> = Vec::new();
            let mut v: Vec<T> = Vec::new();

            unsafe {
                while *p != T::default() {
                    if k.contains(&*p) {
                        return None;
                    }
                    k.push(*p);
                    v.push(*p.add(1));
                    p = p.add(2);
                }
            }

            res.props = k.iter().cloned().zip(v).collect();
        }

        Some(res)
    }
}

impl<T> Default for Properties<T> {
    fn default() -> Self {
        Self { props: Vec::new() }
    }
}
