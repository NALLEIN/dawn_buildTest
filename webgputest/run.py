import SimpleHTTPServer
import socketserver

PORT = 8000

Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
Handler.extensions_map.update({
    ".js": "application/javascript",
})

httpd = socketserver.TCPServer(("",PORT),Handler)

print("Serving at pory", PORT)
print(Handler.extensions_map[".js"])
httpd.serve_forever()