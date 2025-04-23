import threading
from http.server import HTTPServer, BaseHTTPRequestHandler

class HelloHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        port = self.server.server_port 
        print(f"Received request on port {port}")
        
        response = (
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!"
        )

        self.wfile.write(response.encode('utf-8'))

def run_server(port):
    server_address = ('', port)
    httpd = HTTPServer(server_address, HelloHandler)
    print(f"Starting server on port {port}...")
    httpd.serve_forever()

if __name__ == '__main__':
    ports = [9090, 9091, 9092, 9093, 9094, 9095, 9096, 9097, 9098, 9099]  
    threads = []
    for port in ports:
        t = threading.Thread(target=run_server, args=(port,))
        t.daemon = True  
        t.start()
        threads.append(t)
    
    for t in threads:
        t.join()