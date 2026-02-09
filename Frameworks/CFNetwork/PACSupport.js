/*
	File:       PACSuppport.js
 
	Contains:   PAC Support functions
 
	Copyright:  © 2003-2004 by Apple Computer, Inc., all rights reserved
 
	Bugs?:      For bug reports, consult the following page on
				the World Wide Web:
 
					http://developer.apple.com/bugreporter/
 
*/

var __Apple_dsn_cache = nil;

function isPlainHostName(host) {
    
    return (host.indexOf('.') == -1 ? true : false);
}


function dnsDomainIs(host, domain) {
    
    var h = host.toLowerCase();
    var d = domain.toLowerCase();
    var sub = h.substring(h.length - d.length, h.length);

    if (sub == d)
        return true;
    
    return false;
}


function localHostOrDomainIs(host, hostdom) {
    
    var h1 = host.toLowerCase();
    var h2 = hostdom.toLowerCase();
    
    return ((h1 == h2) || (isPlainHostName(h1) & !isPlainHostName(h2))) ? true : false;
}


function isResolvable(host) {
    
    var ip = dnsResolve(host);
    
    return ((typeof ip == "string") && ip.length) ? true : false;
}


function isInNet(host, pattern, mask) {
    
    var ip = dnsResolve(host);
    
    if (ip) {
        
        var p = pattern.split('.');
        var m = mask.split('.');
        var a = ip.split('.');
        
        if ((p.length == m.length) && (m.length == a.length)) {
            
            for (i = 0; i < p.length; i++) {
                if ((p[i]& m[i]) != (m[i]& a[i]))
                    return false;
            }
            
            return true;
        }
    }
    
    return false;
}


function dnsResolve(host) {
    
	var ips, i;
	for (i = 0; i < __Apple_dsn_cache.length; i += 2) {
		
		if (__Apple_dsn_cache[i] == host) {
			ips = __Apple_dsn_cache[i + 1];
			break;
		}
	}
	
	if (i >= __Apple_dsn_cache.length) {
		
		ips = __dnsResolve(host);
		
		if (__Apple_dsn_cache.length == 20) {
			__Apple_dsn_cache.pop();
			__Apple_dsn_cache.pop();
		}
		
		__Apple_dsn_cache[__Apple_dsn_cache.length] = host;
		__Apple_dsn_cache[__Apple_dsn_cache.length] = ips;
	}
    
    if (ips && ips.length) {
        
        for (var i = 0; i < ips.length; i++) {
            
            var bytes = ips[i].split('.');
            if (bytes.length == 4)
                return ips[i];
        }
    }
    
    return null;
}


function myIpAddress() {
    
    var primary = __primaryIPv4Addresses();
    
    var ips = null;
	if (primary && primary.length) {
		ips = primary.Addresses;
	}
    
    if (ips && ips.length) {
        
        for (var i = 0; i < ips.length; i++) {
            
            var bytes = ips[i].split('.');
            if (bytes.length == 4)
                return ips[i];
        }
    }
    
    return null;
}


function dnsDomainLevels(host) {
    
    var parts = host.split('.');
    return parts.length - 1;
}


function shExpMatch(str, shexp) {
    
    if (typeof str != "string" || typeof shexp != "string")
        return false;
    
    if (shexp == "*")
        return true;
    if (str == "" && shexp == "")
        return true;

	str = str.toLowerCase();
	shexp = shexp.toLowerCase();

    var len = str.length;
    var pieces = shexp.split('*');
    var start = 0;
    for (i = 0; i < pieces.length; i++) {
        
        if (pieces[i] == "")
            continue;
        
        if (start > len)
            return false;
        
        start = str.indexOf(pieces[i]);
        if (start == -1)
            return false;
        
        start += pieces[i].length;
		
		str = str.substring(start, len);
		len = str.length;
    }
    
    i--;
    
    if ((pieces[i] == "") || (str == ""))
        return true;
    
    return false;
}


function weekdayRange(wd1, wd2, gmt) {
    
    var today = new Date();
    var days = "SUNMONTUEWEDTHUFRISAT";
    
    wd1 = wd1.toUpperCase();
    if (wd2 == undefined)
        wd2 = wd1;
    else
        wd2 = wd2.toUpperCase();
    
    var d1 = days.indexOf(wd1);
    var d2 = days.indexOf(wd2);
    
    if ((d2 == -1) && (wd2 == "GMT")) {
        gmt = wd2;
        d2 = d1;
    }
    
    if ((d1 == -1) || (d2 == -1))
        return false;
    
    d1 = d1 / 3;
    d2 = d2 / 3;
    
    if (gmt == "GMT")
        today = today.getUTCDay();
    else
        today = today.getDay();
    
    if ((d1 <= d2) && (today >= d1) && (today <= d2))
        return true;
    
    if ((d2 < d1) && ((today <= d2) || (today >= d1)))
        return true;
    
    return false;
}


function dateRange() {
    
    var today = new Date();
    
    var num = arguments.length;
    var gmt = arguments[num - 1];
    if (typeof gmt !=  "string")
        gmt = false;
    else {
        gmt = gmt.toUpperCase();
        if (gmt != "GMT")
            gmt = false;
        else {
            gmt = true;
            num--;
        }
    }
    
    if (!num || (num > 6))
        return false;
    
    var d1 = 0;
    var d2 = 0;
    var m1 = 0;
    var m2 = 0;
    var y1 = 0;
    var y2 = 0;
    
    for (i = 0; i < num; i++) {
        
        var arg = arguments[i];
        
        if (typeof arg == "number") {
            
            if (arg > 31) {
                
                if (!y1)
                    y1 = arg;
                else if (!y2)
                    y2 = arg;
                else
                    return false;
            }
            
            else if (!arg)
                return false;
            
            else if (!d1)
                d1 = arg;
            else if (!d2)
                d2 = arg;
            else
                return false;
        }
        
        else if (typeof arg == "string") {
            
            var months = "JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";
            
            arg = arg.toUpperCase();
            arg = months.indexOf(arg);
            
            if (arg == -1)
                return false;            
            
            arg /= 3;
            arg += 1;
            
            if (!m1)
                m1 = arg;
            else if (!m2)
                m2 = arg;
            else
                return false;
        }
        
        else
            return false;
    }
    
    if (!y1) y1 = gmt ? today.getUTCFullYear() : today.getFullYear();
    if (!y2) y2 = y1;
    if (!m1) m1 = (gmt ? today.getUTCMonth() : today.getMonth()) + 1;
    if (!m2) m2 = m1;
    if (!d1) d1 = gmt ? today.getUTCDate() : today.getDate();
    if (!d2) d2 = d1;
    
    var date1;
    var date2;
    
    if (gmt) {
        
        date1 = Date.UTC(y1, m1 - 1, d1, 0, 0, 0, 0);
        date2 = Date.UTC(y2, m2 - 1, d2, 23, 59, 59, 999);
    }
    
    else {
        
        date1 = (Date(y1, m1 - 1, d1, 0, 0, 0, 0)).valueOf();
        date2 = (Date(y2, m2 - 1, d2, 23, 59, 59, 999)).valueOf();
    }
    
    today = today.valueOf();
    
    return ((date1 <= today) && (today <= date2));
}


function timeRange() {
    
    var date1 = new Date();
    var today = new Date();
    var date2 = new Date();
    
    var num = arguments.length;
    var gmt = arguments[num - 1];
    if (typeof gmt !=  "string")
        gmt = false;
    else {
        gmt = gmt.toUpperCase();
        if (gmt != "GMT")
            gmt = false;
        else {
            gmt = true;
            num--;
        }
    }
    
    if (!num || (num > 6) || ((num % 2) && (num != 1)))
        return false;
    
    date1.setMinutes(0);
    date1.setSeconds(0);
    date1.setMilliseconds(0);
    
    date2.setMinutes(59);
    date2.setSeconds(59);
    date2.setMilliseconds(999);
    
    for (i = 0; i < (num / 2); i++) {
        
        var arg = arguments[i];
        if (gmt) {
            switch (i) {
                case 0:
                    date1.setUTCHours(arg);
                    date2.setUTCHours(arg);
                    break;
                case 1:
                    date1.setUTCMinutes(arg);
                    date2.setUTCMinutes(arg);
                    break;
                case 2:
                    date1.setUTCSeconds(arg);
                    date2.setUTCSeconds(arg);
                    break;
            }
        }
        else {
            switch (i) {
                case 0:
                    date1.setHours(arg);
                    date2.setHours(arg);
                    break;
                case 1:
                    date1.setMinutes(arg);
                    date2.setMinutes(arg);
                    break;
                case 2:
                    date1.setSeconds(arg);
                    date2.setSeconds(arg);
                    break;
            }
        }
    }
    
    if (num != 1) {
        
        date2.setMinutes(0);
        date2.setSeconds(0);
        date2.setMilliseconds(0);
        
        for (i = 0; i < (num / 2); i++) {
            
            var arg = arguments[(num / 2) + i];
            if (gmt) {
                switch (i) {
                    case 0:
                        date2.setUTCHours(arg);
                        break;
                    case 1:
                        date2.setUTCMinutes(arg);
                        break;
                    case 2:
                        date2.setUTCSeconds(arg);
                        break;
                }
            }
            else {
                switch (i) {
                    case 0:
                        date2.setHours(arg);
                        break;
                    case 1:
                        date2.setMinutes(arg);
                        break;
                    case 2:
                        date2.setSeconds(arg);
                        break;
                }
            }
        }
    }
    
    today = today.valueOf();
    date1 = date1.valueOf();
    date2 = date2.valueOf();
    
    if (date2 < date1)
        date2 += 86400000;
    
    return ((date1 <= today) && (today <= date2));
}


function __Apple_FindProxyForURL(host, url) {
	
	__Apple_dsn_cache = new Array;
	
	var result = FindProxyForURL(host, url);
	
	__Apple_dsn_cache = new Array;
	
	return result;
}
