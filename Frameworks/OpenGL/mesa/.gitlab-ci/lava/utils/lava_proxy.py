import time
import traceback
import urllib
import urllib.parse
import xmlrpc
import xmlrpc.client

import lavacli

from .log_follower import fatal_err, print_log


def setup_lava_proxy():
    config = lavacli.load_config("default")
    uri, usr, tok = (config.get(key) for key in ("uri", "username", "token"))
    uri_obj = urllib.parse.urlparse(uri)
    uri_str = f"{uri_obj.scheme}://{usr}:{tok}@{uri_obj.netloc}{uri_obj.path}"
    transport = lavacli.RequestsTransport(
        uri_obj.scheme,
        config.get("proxy"),
        config.get("timeout", 120.0),
        config.get("verify_ssl_cert", True),
    )
    proxy = xmlrpc.client.ServerProxy(uri_str, allow_none=True, transport=transport)

    print_log(f'Proxy for {config["uri"]} created.')

    return proxy


def call_proxy(fn, *args):
    retries = 60
    for n in range(1, retries + 1):
        try:
            return fn(*args)
        except xmlrpc.client.ProtocolError as err:
            if n == retries:
                traceback.print_exc()
                fatal_err(f"A protocol error occurred (Err {err.errcode} {err.errmsg})")
            else:
                time.sleep(15)
        except xmlrpc.client.Fault as err:
            traceback.print_exc()
            fatal_err(f"FATAL: Fault: {err.faultString} (code: {err.faultCode})", err)
