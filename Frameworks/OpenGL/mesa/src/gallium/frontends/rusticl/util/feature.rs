#[macro_export]
macro_rules! has_required_feature {
    ($object:ident, $feature:ident) => {{
        let has_feature = $object.$feature.is_some();
        if !has_feature {
            println!(
                "Missing {} feature {}",
                stringify!($object),
                stringify!($feature)
            );
        }
        has_feature
    }};
}
