/^ *T("/{
    s/^[^"]*"/    z_/
    s/".*$/,/
    s/-//g
    s/\./D/g
    P
    s/    z_\(.*\),/#define t_\1 (\&thingies[z_\1])/
    P
}
