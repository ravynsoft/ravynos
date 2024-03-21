/^ *W(/{
    s/[^,]*, *t_/    wi_/
    s/ *,.*/,/
    P
    s/    wi_\(.*\),/#define w_\1 (\&widgets[wi_\1])/
    P
}
