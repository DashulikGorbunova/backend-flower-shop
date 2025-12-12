#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define closesocket close
#endif

class SimpleServer {
private:
    SOCKET server_fd;
    int port;
    
public:
    SimpleServer(int port = 8080) : port(port), server_fd(-1) {}
    
    bool start() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
#endif
        
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
        
        // Allow port reuse
        int opt = 1;
#ifdef _WIN32
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed on port " << port << std::endl;
            return false;
        }
        
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return false;
        }
        
        std::cout << "✅ Сервер запущен на порту " << port << std::endl;
        std::cout << "🌐 Доступен по: http://localhost:" << port << std::endl;
        
        return true;
    }
    
    void handleClient(SOCKET client_socket) {
        char buffer[1024] = {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        
        std::string request(buffer);
        std::string response;
        
        if (request.find("GET / ") != std::string::npos || 
            request.find("GET /index.html") != std::string::npos) {
            response = serveFile("templates/index.html");
        }
        else if (request.find("GET /bouquets") != std::string::npos) {
            response = serveFile("templates/bouquets.html");
        }
        else if (request.find("GET /cart") != std::string::npos) {
            response = serveFile("templates/cart.html");
        }
        else if (request.find("GET /api/health") != std::string::npos) {
            response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/json\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "\r\n"
                      "{\"status\":\"ok\",\"service\":\"flower-shop\"}";
        }
        else if (request.find("POST /api/order") != std::string::npos) {
            response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/json\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "\r\n"
                      "{\"success\":true,\"message\":\"Заказ принят!\"}";
        }
        else if (request.find(".css") != std::string::npos) {
            std::string filename = extractFilename(request);
            response = serveStaticFile(filename, "text/css");
        }
        else if (request.find(".js") != std::string::npos) {
            std::string filename = extractFilename(request);
            response = serveStaticFile(filename, "application/javascript");
        }
        else if (request.find(".jpg") != std::string::npos || 
                 request.find(".png") != std::string::npos) {
            std::string filename = extractFilename(request);
            response = serveStaticFile(filename, "image/jpeg");
        }
        else {
            response = serveFile("templates/index.html");
        }
        
        send(client_socket, response.c_str(), response.length(), 0);
        closesocket(client_socket);
    }
    
    void run() {
        while (true) {
            sockaddr_in client_addr;
#ifdef _WIN32
            int addrlen = sizeof(client_addr);
#else
            socklen_t addrlen = sizeof(client_addr);
#endif
            
            SOCKET client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
            
            if (client_socket < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            
            // Обрабатываем в отдельном потоке
            std::thread([this, client_socket]() {
                handleClient(client_socket);
            }).detach();
        }
    }
    
private:
    std::string extractFilename(const std::string& request) {
        size_t start = request.find("GET ") + 4;
        size_t end = request.find(" HTTP/");
        std::string path = request.substr(start, end - start);
        return path;
    }
    
    std::string serveStaticFile(const std::string& filename, const std::string& contentType) {
        std::string fullpath = "static" + filename;
        std::ifstream file(fullpath, std::ios::binary);
        
        if (!file.is_open()) {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        std::string content = buffer.str();
        
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " + contentType + "\r\n"
               "Content-Length: " + std::to_string(content.length()) + "\r\n"
               "\r\n" + content;
    }
    
    std::string serveFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        std::string content = buffer.str();
        
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(content.length()) + "\r\n"
               "\r\n" + content;
    }
};

int main() {
    // Получаем порт из переменной окружения (для Railway/Render)
    const char* env_port = std::getenv("PORT");
    int port = env_port ? std::atoi(env_port) : 8080;
    
    SimpleServer server(port);
    
    if (!server.start()) {
        std::cerr << "❌ Не удалось запустить сервер" << std::endl;
        return 1;
    }
    
    server.run();
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}