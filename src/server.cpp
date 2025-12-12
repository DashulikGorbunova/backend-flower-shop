#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

struct Product {
    int id;
    std::string name;
    double price;
    std::string category;
    std::string image;
    std::string description;
};

struct CustomBouquet {
    int id;
    std::string name;
    double price;
    std::string description;
    std::string image;
};

class FlowerShopServer {
private:
    SOCKET serverSocket;
    std::vector<Product> products;
    std::vector<Product> cart;
    std::vector<CustomBouquet> customBouquets;
    int nextCustomBouquetId;
    
public:
    FlowerShopServer() : serverSocket(INVALID_SOCKET), nextCustomBouquetId(1000) {
    products = {
        // Цветы для каталога (букеты)
        {1, "Букет красных роз", 1500.0, "flowers", "/static/images/flowers/red-roses.jpg", "Свежие красные розы премиум качества, 7 шт в букете"},
        {2, "Букет белых тюльпанов", 800.0, "flowers", "/static/images/flowers/white-tulips.jpg", "Нежные белые тюльпаны из Голландии, 10 шт в букете"},
        {3, "Орхидеи фаленопсис", 2500.0, "flowers", "/static/images/flowers/orchids.jpg", "Экзотические орхидеи в горшках"},
        {4, "Букет розовых пионов", 1800.0, "flowers", "/static/images/flowers/peonies.jpg", "Пышные розовые пионы, 5 шт в букете"},
        {5, "Букет подсолнухов", 700.0, "flowers", "/static/images/flowers/sunflowers.jpg", "Яркие солнечные подсолнухи, 7 шт в букете"},
        {6, "Букет лилий", 1200.0, "flowers", "/static/images/flowers/lilies.jpg", "Ароматные белые лилии, 5 шт в букете"},
        {7, "Букет ирисов", 600.0, "flowers", "/static/images/flowers/irises.jpg", "Фиолетовые ирисы, 10 шт в букете"},
        {8, "Букет хризантем", 900.0, "flowers", "/static/images/flowers/chrysanthemums.jpg", "Белые хризантемы, 9 шт в букете"},
        {9, "Букет гербер", 850.0, "flowers", "/static/images/flowers/gerberas.jpg", "Яркие разноцветные герберы, 7 шт в букете"},
        {10, "Букет ландышей", 950.0, "flowers", "/static/images/flowers/lilies-of-valley.jpg", "Нежные ландыши, 15 шт в букете"},
        {11, "Букет гортензий", 1600.0, "flowers", "/static/images/flowers/hydrangeas.jpg", "Пышные голубые гортензии, 3 шт в букете"},
        {12, "Букет альстромерий", 750.0, "flowers", "/static/images/flowers/alstroemerias.jpg", "Пестрые альстромерии, 7 шт в букете"},
        
        // Цветы для конструктора (поштучно)
        {101, "Красная роза", 250.0, "builder_flowers", "/static/images/flowers/red-roses.jpg", "Свежая красная роза премиум качества, длина стебля 50см"},
        {102, "Белый тюльпан", 80.0, "builder_flowers", "/static/images/flowers/white-tulips.jpg", "Нежный белый тюльпан из Голландии, свежий срез"},
        {103, "Орхидея фаленопсис", 2500.0, "builder_flowers", "/static/images/flowers/orchids.jpg", "Экзотическая орхидея в горшке, цветение 2-3 месяца"},
        {104, "Розовый пион", 400.0, "builder_flowers", "/static/images/flowers/peonies.jpg", "Пышный розовый пион, диаметр цветка 12-15см"},
        {105, "Подсолнух", 120.0, "builder_flowers", "/static/images/flowers/sunflowers.jpg", "Яркий солнечный подсолнух, диаметр 15-20см"},
        {106, "Лилия", 280.0, "builder_flowers", "/static/images/flowers/lilies.jpg", "Ароматная белая лилия, 3-5 бутонов на стебле"},
        {107, "Ирис", 70.0, "builder_flowers", "/static/images/flowers/irises.jpg", "Фиолетовый ирис, нежный весенний цветок"},
        {108, "Хризантема", 110.0, "builder_flowers", "/static/images/flowers/chrysanthemums.jpg", "Белая хризантема, кустовая, 5-7 соцветий"},
        {109, "Гербера", 130.0, "builder_flowers", "/static/images/flowers/gerberas.jpg", "Яркая разноцветная гербера, диаметр 10-12см"},
        {110, "Ландыш", 65.0, "builder_flowers", "/static/images/flowers/lilies-of-valley.jpg", "Нежный ландыш, веточка с 8-10 цветками"},
        {111, "Гортензия", 550.0, "builder_flowers", "/static/images/flowers/hydrangeas.jpg", "Пышная голубая гортензия, крупное соцветие"},
        {112, "Альстромерия", 110.0, "builder_flowers", "/static/images/flowers/alstroemerias.jpg", "Пестрая альстромерия, 5-7 цветков на стебле"},
        
        // Готовые букеты
        {13, "Романтический букет", 3200.0, "bouquets", "/static/images/bouquets/romantic.jpg", "Букет из красных роз и гипсофилы"},
        {14, "Свадебный букет", 4500.0, "bouquets", "/static/images/bouquets/wedding.jpg", "Элегантный свадебный букет невесты"},
        {15, "Букет для мамы", 2800.0, "bouquets", "/static/images/bouquets/for-mom.jpg", "Яркий и нежный букет из роз и хризантем"},
        {16, "Деловой букет", 3800.0, "bouquets", "/static/images/bouquets/business.jpg", "Строгий букет для делового партнера"},
        {17, "Весенний микс", 2900.0, "bouquets", "/static/images/bouquets/spring-mix.jpg", "Свежий весенний букет из тюльпанов и ирисов"},
        {18, "Осенняя композиция", 3500.0, "bouquets", "/static/images/bouquets/autumn.jpg", "Теплый осенний букет с подсолнухами"},
        {19, "Летнее настроение", 2700.0, "bouquets", "/static/images/bouquets/summer.jpg", "Яркий летний букет из гербер и ромашек"},
        {20, "Зимняя сказка", 4200.0, "bouquets", "/static/images/bouquets/winter.jpg", "Белый зимний букет с хризантемами и розами"},
        {21, "Девиччий букет", 2300.0, "bouquets", "/static/images/bouquets/girly.jpg", "Нежный букет в пастельных тонах"},
        {22, "Мужской букет", 3100.0, "bouquets", "/static/images/bouquets/masculine.jpg", "Сдержанный букет в синих тонах"},
        {23, "Юбилейный букет", 4800.0, "bouquets", "/static/images/bouquets/anniversary.jpg", "Роскошный букет для особого случая"},
        {24, "Букет на первое свидание", 2600.0, "bouquets", "/static/images/bouquets/first-date.jpg", "Романтичный букет для свидания"}
    };
}

    bool initialize() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
        
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);
        
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            return false;
        }
        
        if (listen(serverSocket, 10) == SOCKET_ERROR) {
            std::cerr << "Listen failed" << std::endl;
            return false;
        }
        
        std::cout << "Магазин цветов запущен!" << std::endl;
        std::cout << "http://localhost:8080" << std::endl;
        return true;
    }
    
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return "";
        std::string content((std::istreambuf_iterator<char>(file)), 
                           std::istreambuf_iterator<char>());
        return content;
    }
    
    std::string getMimeType(const std::string& filename) {
        if (filename.find(".html") != std::string::npos) return "text/html; charset=utf-8";
        if (filename.find(".css") != std::string::npos) return "text/css";
        if (filename.find(".js") != std::string::npos) return "application/javascript";
        if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos) return "image/jpeg";
        if (filename.find(".png") != std::string::npos) return "image/png";
        if (filename.find(".gif") != std::string::npos) return "image/gif";
        if (filename.find(".webp") != std::string::npos) return "image/webp";
        if (filename.find(".svg") != std::string::npos) return "image/svg+xml";
        return "text/plain";
    }
    
    std::string createHTTPResponse(const std::string& content, const std::string& mimeType = "text/html", int statusCode = 200) {
        std::stringstream response;
        response << "HTTP/1.1 " << statusCode << " OK\r\n";
        response << "Content-Type: " << mimeType << "\r\n";
        response << "Content-Length: " << content.length() << "\r\n";
        response << "Connection: close\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "\r\n";
        response << content;
        return response.str();
    }
    
    std::string escapeJsonString(const std::string& input) {
        std::string output;
        for (char c : input) {
            if (c == '"') output += "\\\"";
            else if (c == '\\') output += "\\\\";
            else output += c;
        }
        return output;
    }
    
    std::string extractRequestBody(const std::string& request) {
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            return request.substr(bodyStart + 4);
        }
        return "";
    }
    
    std::string productsToJSON() {
        std::stringstream json;
        json << "{\"products\":[";
        for (size_t i = 0; i < products.size(); ++i) {
            json << "{";
            json << "\"id\":" << products[i].id << ",";
            json << "\"name\":\"" << escapeJsonString(products[i].name) << "\",";
            json << "\"price\":" << products[i].price << ",";
            json << "\"category\":\"" << escapeJsonString(products[i].category) << "\",";
            json << "\"image\":\"" << escapeJsonString(products[i].image) << "\",";
            json << "\"description\":\"" << escapeJsonString(products[i].description) << "\"";
            json << "}";
            if (i < products.size() - 1) json << ",";
        }
        json << "]}";
        return json.str();
    }
    
    void addToCart(int productId) {
        for (const auto& product : products) {
            if (product.id == productId) {
                cart.push_back(product);
                break;
            }
        }
    }
    
    void addCustomBouquet(const std::string& jsonData) {
    try {
        std::cout << "=== ДЕБАГ: Полученные данные ===" << std::endl;
        std::cout << jsonData << std::endl;
        
        // Простой JSON парсинг
        std::string name = "Кастомный букет";
        double price = 0;
        std::string description = "Собранный букет";
        
        // Ищем поля в JSON
        size_t name_start = jsonData.find("\"name\":\"");
        if (name_start != std::string::npos) {
            name_start += 8; // Длина "\"name\":\""
            size_t name_end = jsonData.find("\"", name_start);
            if (name_end != std::string::npos) {
                name = jsonData.substr(name_start, name_end - name_start);
                // Заменяем проблемные символы
                std::replace(name.begin(), name.end(), '+', ' ');
            }
        }
        
        // Парсим цену
        size_t price_start = jsonData.find("\"price\":");
        if (price_start != std::string::npos) {
            price_start += 8; // Длина "\"price\":"
            size_t price_end = jsonData.find_first_of(",}", price_start);
            if (price_end != std::string::npos) {
                std::string price_str = jsonData.substr(price_start, price_end - price_start);
                // Удаляем все нечисловые символы кроме точки и цифр
                price_str.erase(std::remove_if(price_str.begin(), price_str.end(), 
                    [](char c) { return !std::isdigit(c) && c != '.'; }), price_str.end());
                
                if (!price_str.empty()) {
                    price = std::stod(price_str);
                }
            }
        }
        
        // Парсим описание
        size_t desc_start = jsonData.find("\"description\":\"");
        if (desc_start != std::string::npos) {
            desc_start += 15; // Длина "\"description\":\""
            size_t desc_end = jsonData.find("\"", desc_start);
            if (desc_end != std::string::npos) {
                description = jsonData.substr(desc_start, desc_end - desc_start);
            }
        }
        
        std::cout << "Распарсено: name='" << name << "', price=" << price << ", desc='" << description << "'" << std::endl;
        
        CustomBouquet bouquet;
        bouquet.id = nextCustomBouquetId++;
        bouquet.name = name;
        bouquet.price = price;
        bouquet.description = description;
        bouquet.image = "💐"; // Используем эмодзи вместо картинки
        
        customBouquets.push_back(bouquet);
        std::cout << "=== Букет успешно добавлен ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "ОШИБКА ПАРСИНГА: " << e.what() << std::endl;
        
        // Добавляем букет с базовыми значениями
        CustomBouquet bouquet;
        bouquet.id = nextCustomBouquetId++;
        bouquet.name = "Кастомный букет";
        bouquet.price = 0;
        bouquet.description = "Собранный букет";
        bouquet.image = "💐";
        
        customBouquets.push_back(bouquet);
    }
}
    
    std::string getCartJSON() {
        std::stringstream json;
        json << "{\"cart\":[";
        
        // Обычные товары
        for (size_t i = 0; i < cart.size(); ++i) {
            json << "{";
            json << "\"id\":" << cart[i].id << ",";
            json << "\"name\":\"" << escapeJsonString(cart[i].name) << "\",";
            json << "\"price\":" << cart[i].price << ",";
            json << "\"image\":\"" << escapeJsonString(cart[i].image) << "\",";
            json << "\"description\":\"" << escapeJsonString(cart[i].description) << "\"";
            json << "}";
            if (i < cart.size() - 1 || !customBouquets.empty()) json << ",";
        }
        
        // Кастомные букеты
        for (size_t i = 0; i < customBouquets.size(); ++i) {
            json << "{";
            json << "\"id\":" << customBouquets[i].id << ",";
            json << "\"name\":\"" << escapeJsonString(customBouquets[i].name) << "\",";
            json << "\"price\":" << customBouquets[i].price << ",";
            json << "\"image\":\"" << escapeJsonString(customBouquets[i].image) << "\",";
            json << "\"description\":\"" << escapeJsonString(customBouquets[i].description) << "\"";
            json << "}";
            if (i < customBouquets.size() - 1) json << ",";
        }
        
        json << "],\"total\":" << calculateTotal() << "}";
        return json.str();
    }
    
    double calculateTotal() {
        double total = 0;
        for (const auto& item : cart) {
            total += item.price;
        }
        for (const auto& bouquet : customBouquets) {
            total += bouquet.price;
        }
        return total;
    }
    
    void clearCart() {
        cart.clear();
        customBouquets.clear();
    }
    
    void handleRequest(SOCKET clientSocket) {
        char buffer[8192];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string request(buffer);
            
            std::string method, path;
            std::istringstream iss(request);
            iss >> method >> path;
            
            size_t questionMark = path.find('?');
            if (questionMark != std::string::npos) {
                path = path.substr(0, questionMark);
            }
            
            std::cout << method << " " << path << std::endl;
            
            std::string response;
            
            if (path == "/") {
                response = createHTTPResponse(getHomePage());
            }
            else if (path == "/catalog") {
                response = createHTTPResponse(getCatalogPage());
            }
            else if (path == "/bouquets") {
                response = createHTTPResponse(getBouquetsPage());
            }
            else if (path == "/builder") {
                response = createHTTPResponse(getBuilderPage());
            }
            else if (path == "/cart") {
                response = createHTTPResponse(getCartPage());
            }
            else if (path == "/api/products") {
                response = createHTTPResponse(productsToJSON(), "application/json");
            }
            else if (path == "/api/cart") {
                response = createHTTPResponse(getCartJSON(), "application/json");
            }
            else if (path.find("/api/cart/add/") == 0) {
                try {
                    int productId = std::stoi(path.substr(14));
                    addToCart(productId);
                    response = createHTTPResponse("{\"status\":\"success\"}", "application/json");
                } catch (...) {
                    response = createHTTPResponse("{\"status\":\"error\"}", "application/json", 400);
                }
            }
            else if (path == "/api/cart/custom" && method == "POST") {
                std::string body = extractRequestBody(request);
                if (!body.empty()) {
                    addCustomBouquet(body);
                    response = createHTTPResponse("{\"status\":\"success\"}", "application/json");
                } else {
                    response = createHTTPResponse("{\"status\":\"error\"}", "application/json", 400);
                }
            }
            else if (path == "/api/cart/clear") {
                clearCart();
                response = createHTTPResponse("{\"status\":\"success\"}", "application/json");
            }
            else if (path.find("/static/") == 0) {
                std::string filepath = path.substr(1);
                std::string content = readFile(filepath);
                if (!content.empty()) {
                    response = createHTTPResponse(content, getMimeType(filepath));
                } else {
                    response = createHTTPResponse("<h1>404</h1>", "text/html", 404);
                }
            }
            else {
                response = createHTTPResponse("<h1>404</h1>", "text/html", 404);
            }
            
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        closesocket(clientSocket);
    }
    
    std::string getHomePage() {
        return "<!DOCTYPE html>"
               "<html lang=\"ru\">"
               "<head>"
               "<meta charset=\"UTF-8\">"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<title>Магазин цветов</title>"
               "<link rel=\"stylesheet\" href=\"/static/css/style.css\">"
               "</head>"
               "<body>"
               "<div class=\"container\">"
               "<header>"
               "<div class=\"logo\">Flora</div>"
               "<nav>"
               "<a href=\"/\" class=\"nav-link active\">Главная</a>"
               "<a href=\"/catalog\" class=\"nav-link\">Цветы</a>"
               "<a href=\"/bouquets\" class=\"nav-link\">Букеты</a>"
               "<a href=\"/builder\" class=\"nav-link\">Конструктор</a>"
               "<a href=\"/cart\" class=\"nav-link cart-link\">Корзина</a>"
               "</nav>"
               "</header>"
               "<main>"
               "<section class=\"hero\">"
               "<h1>Свежие цветы с доставкой</h1>"
               "<p>Создаём красивые букеты для ваших особенных моментов</p>"
               "<a href=\"/catalog\" class=\"cta-button\">Смотреть каталог</a>"
               "</section>"
               "</main>"
               "</div>"
               "<script src=\"/static/js/app.js\"></script>"
               "</body>"
               "</html>";
    }
    
    std::string getCatalogPage() {
        return "<!DOCTYPE html>"
               "<html lang=\"ru\">"
               "<head>"
               "<meta charset=\"UTF-8\">"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<title>Цветы - Магазин цветов</title>"
               "<link rel=\"stylesheet\" href=\"/static/css/style.css\">"
               "</head>"
               "<body>"
               "<div class=\"container\">"
               "<header>"
               "<div class=\"logo\">Flora</div>"
               "<nav>"
               "<a href=\"/\" class=\"nav-link\">Главная</a>"
               "<a href=\"/catalog\" class=\"nav-link active\">Цветы</a>"
               "<a href=\"/bouquets\" class=\"nav-link\">Букеты</a>"
               "<a href=\"/builder\" class=\"nav-link\">Конструктор</a>"
               "<a href=\"/cart\" class=\"nav-link cart-link\">Корзина</a>"
               "</nav>"
               "</header>"
               "<main>"
               "<h1>Цветы</h1>"
               "<div class=\"products-grid\" id=\"products\"></div>"
               "</main>"
               "</div>"
               "<script src=\"/static/js/app.js\"></script>"
               "<script>loadProducts('flowers');</script>"
               "</body>"
               "</html>";
    }
    
    std::string getBouquetsPage() {
        return "<!DOCTYPE html>"
               "<html lang=\"ru\">"
               "<head>"
               "<meta charset=\"UTF-8\">"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<title>Букеты - Магазин цветов</title>"
               "<link rel=\"stylesheet\" href=\"/static/css/style.css\">"
               "</head>"
               "<body>"
               "<div class=\"container\">"
               "<header>"
               "<div class=\"logo\">Flora</div>"
               "<nav>"
               "<a href=\"/\" class=\"nav-link\">Главная</a>"
               "<a href=\"/catalog\" class=\"nav-link\">Цветы</a>"
               "<a href=\"/bouquets\" class=\"nav-link active\">Букеты</a>"
               "<a href=\"/builder\" class=\"nav-link\">Конструктор</a>"
               "<a href=\"/cart\" class=\"nav-link cart-link\">Корзина</a>"
               "</nav>"
               "</header>"
               "<main>"
               "<h1>Готовые букеты</h1>"
               "<div class=\"products-grid\" id=\"products\"></div>"
               "</main>"
               "</div>"
               "<script src=\"/static/js/app.js\"></script>"
               "<script>loadProducts('bouquets');</script>"
               "</body>"
               "</html>";
    }
    
    std::string getBuilderPage() {
    return "<!DOCTYPE html>"
           "<html lang=\"ru\">"
           "<head>"
           "<meta charset=\"UTF-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
           "<title>Конструктор букетов - Магазин цветов</title>"
           "<link rel=\"stylesheet\" href=\"/static/css/style.css\">"
           "</head>"
           "<body>"
           "<div class=\"container\">"
           "<header>"
           "<div class=\"logo\">Flora</div>"
           "<nav>"
           "<a href=\"/\" class=\"nav-link\">Главная</a>"
           "<a href=\"/catalog\" class=\"nav-link\">Цветы</a>"
           "<a href=\"/bouquets\" class=\"nav-link\">Букеты</a>"
           "<a href=\"/builder\" class=\"nav-link active\">Конструктор</a>"
           "<a href=\"/cart\" class=\"nav-link cart-link\">Корзина</a>"
           "</nav>"
           "</header>"
           "<main>"
           "<h1>Конструктор букетов</h1>"
           "<div id=\"debug-info\" style=\"display: none; background: #f8f9fa; padding: 1rem; margin-bottom: 1rem; border-radius: 4px;\"></div>"
           "<div class=\"builder-container\">"
           "<div class=\"builder-sidebar\">"
           "<h3>Выберите цветы (поштучно):</h3>"
           "<div id=\"loading-message\" style=\"text-align: center; padding: 2rem;\">"
           "<div style=\"font-size: 2rem; margin-bottom: 1rem;\">⏳</div>"
           "<p>Загрузка цветов...</p>"
           "</div>"
           "<div id=\"flowers-list\"></div>"
           "</div>"
           "<div class=\"builder-preview\">"
           "<h3>Ваш букет:</h3>"
           "<div id=\"current-bouquet\">"
           "<div class=\"empty-bouquet\">"
           "<div style=\"font-size: 3rem; margin-bottom: 1rem;\">💐</div>"
           "<h3>Букет пуст</h3>"
           "<p>Добавьте цветы из списка слева</p>"
           "</div>"
           "</div>"
           "<div class=\"bouquet-total\">"
           "<strong>Итого: <span id=\"bouquet-price\">0</span> руб.</strong>"
           "<button onclick=\"addBouquetToCart()\" class=\"cta-button\">Добавить букет в корзину</button>"
           "<button onclick=\"clearBouquetBuilder()\" class=\"btn-secondary\" style=\"margin-top: 10px;\">Очистить конструктор</button>"
           "</div>"
           "</div>"
           "</div>"
           "</main>"
           "</div>"
           "<script src=\"/static/js/app.js\"></script>"
           "<script>"
           "console.log('Запуск конструктора...');"
           "setTimeout(() => { loadBuilderFlowers(); }, 100);"
           "</script>"
           "</body>"
           "</html>";
}
    
    std::string getCartPage() {
        return "<!DOCTYPE html>"
               "<html lang=\"ru\">"
               "<head>"
               "<meta charset=\"UTF-8\">"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<title>Корзина - Магазин цветов</title>"
               "<link rel=\"stylesheet\" href=\"/static/css/style.css\">"
               "</head>"
               "<body>"
               "<div class=\"container\">"
               "<header>"
               "<div class=\"logo\">Flora</div>"
               "<nav>"
               "<a href=\"/\" class=\"nav-link\">Главная</a>"
               "<a href=\"/catalog\" class=\"nav-link\">Цветы</a>"
               "<a href=\"/bouquets\" class=\"nav-link\">Букеты</a>"
               "<a href=\"/builder\" class=\"nav-link\">Конструктор</a>"
               "<a href=\"/cart\" class=\"nav-link active cart-link\">Корзина</a>"
               "</nav>"
               "</header>"
               "<main>"
               "<h1>Корзина</h1>"
               "<div id=\"cart-items\"></div>"
               "<div class=\"cart-total\">"
               "<h3>Общая сумма: <span id=\"total-price\">0</span> руб.</h3>"
               "<button onclick=\"clearCart()\" class=\"btn-secondary\">Очистить корзину</button>"
               "<button onclick=\"checkout()\" class=\"cta-button\">Оформить заказ</button>"
               "</div>"
               "</main>"
               "</div>"
               "<script src=\"/static/js/app.js\"></script>"
               "<script>loadCart();</script>"
               "</body>"
               "</html>";
    }
    
    void run() {
        while (true) {
            SOCKET clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket != INVALID_SOCKET) {
                handleRequest(clientSocket);
            }
        }
    }
    
    ~FlowerShopServer() {
        if (serverSocket != INVALID_SOCKET) closesocket(serverSocket);
        WSACleanup();
    }
};

int main() {
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::atoi(port_env) : 8080;
    
    std::cout << "Сервер запущен на порту: " << port << std::endl;

    FlowerShopServer server;
    if (server.initialize()) {
        server.run();
    }
    return 0;
}