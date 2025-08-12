from http.server import BaseHTTPRequestHandler, HTTPServer
import requests

BACKEND_SERVER = "http://192.168.10.2:8080"

class ProxyHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            resp = requests.get(f"{BACKEND_SERVER}{self.path}", timeout=5)
            self.send_response(resp.status_code)
            self.send_header('Content-type', resp.headers.get('Content-type', 'text/html'))
            self.end_headers()
            self.wfile.write(resp.content)
        except requests.exceptions.RequestException as e:
            print(f"Error menghubungi backend: {e}")
            self.send_error(502, "proxy error")

if __name__ == '__main__':
    server_add = ('0.0.0.0', 80)
    httpd = HTTPServer(server_add, ProxyHTTPRequestHandler)
    httpd.serve_forever()