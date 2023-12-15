#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <curl/curl.h>
#include <unistd.h>
#include <cmath>

class Colors{
private:
    std::string red = "\x1B[31m";
    std::string green = "\x1B[32m";
    std::string yellow = "\x1B[33m";
    std::string blue = "\x1B[34m";
    std::string purple = "\x1B[35m";
    std::string end = "\033[0m";
public:
    std::string red_text(const std::string& text) {
        return this->red + text + this->end;
    }
    std::string green_text(const std::string& text) {
        return this->green + text + this->end;
    }
    std::string yellow_text(const std::string& text) {
        return this->yellow + text + this->end;
    }
    std::string blue_text(const std::string& text) {
        return this->blue + text + this->end;
    }
    std::string purple_text(const std::string& text) {
        return this->purple + text + this->end;
    }
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

double MeasureDownloadSpeed(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        auto start_time = std::chrono::high_resolution_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end_time = std::chrono::high_resolution_clock::now();

        double download_speed = 0.0;

        if (res != CURLE_OK) {
            std::cerr << "Failed to download: " << curl_easy_strerror(res) << std::endl;
        } else {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            download_speed = (response.size() / 1024.0) / (duration.count() / 1000.0);
        }

        curl_easy_cleanup(curl);
        return download_speed;
    }

    return 0.0;
}

double MeasurePing(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Perform a HEAD request to reduce data transfer

        auto start_time = std::chrono::high_resolution_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end_time = std::chrono::high_resolution_clock::now();

        double ping_time = 0.0;

        if (res != CURLE_OK) {
            std::cerr << "Failed to ping: " << curl_easy_strerror(res) << std::endl;
        } else {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            ping_time = duration.count();
        }

        curl_easy_cleanup(curl);
        return ping_time;
    }

    return 0.0;
}

void PrintProgressBar(double progress, int barWidth = 50) {
    std::string fill = "▒";
    std::string empty = " ";
    Colors colors;
    int pos = barWidth * progress;
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) {
            if (progress <= 0.25) {
                std::cout << colors.red_text(fill);
            } else if (progress <= 0.75) {
                std::cout << colors.yellow_text(fill);
            } else {
                std::cout << colors.green_text(fill);
            }
        } else {
            std::cout << colors.red_text(empty);
        }
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void smoothPrint(const std::string& text) {
    std::chrono::milliseconds timespan(25);

    std::string res = "";

    for (const char c : text) {
        res += c;
        std::this_thread::sleep_for(timespan);
        std::cout << res << "▁" << "\r" << std::flush;
    }

    std::cout << res << " " << std::endl;
}

int main() {
    std::cout << "\x1B[2J\x1B[H";

    Colors colors;

    smoothPrint(colors.purple_text("SpeedTest by PyWebSol"));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminal_width = w.ws_col;
    int terminal_height = w.ws_row;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    const std::string url = "https://yandex.ru/internet";

    double download_speed;
    double ping_time;

    double total_speed = 0.0;
    double total_ping = 0.0;

    int times = 10;

    for (int i = 0; i < times; i++) {
        double speed = MeasureDownloadSpeed(url);
        double ping = MeasurePing(url);

        total_speed += speed / 2;
        total_ping += ping / 2;

        double progress = static_cast<double>(i + 1) / times;
        PrintProgressBar(progress, 20);
    }

    std::string rmPbar = "";
    for (int _ = 0; _ < terminal_width; _++) {
        rmPbar += " ";
    }
    std::cout << rmPbar << std::endl;

    curl_global_cleanup();

    total_speed = total_speed / times;
    total_ping = total_ping / times;

    if (total_speed <= 16) {
        std::cout << "Скорость загрузки: " << colors.red_text(std::to_string(total_speed)) << " Mbps" << std::endl;
    } else if (total_speed > 16 && total_speed <= 54) {
        std::cout << "Скорость загрузки: " << colors.yellow_text(std::to_string(total_speed)) << " Mbps" << std::endl;
    } else if (total_speed > 54 && total_speed <= 200) {
        std::cout << "Скорость загрузки: " << colors.green_text(std::to_string(total_speed)) << " Mbps" << std::endl;
    } else {
        std::cout << "Скорость загрузки: " << colors.green_text(std::to_string(total_speed)) << " Mbps" << std::endl;
    }

    if (total_ping <= 30) {
        std::cout << "Пинг: " << colors.green_text(std::to_string(total_ping)) << " ms" << std::endl;
    } else if (total_ping > 30 & total_ping <= 150) {
        std::cout << "Пинг: " << colors.yellow_text(std::to_string(total_ping)) << " ms" << std::endl;
    } else {
        std::cout << "Пинг: " << colors.red_text(std::to_string(total_ping)) << " ms" << std::endl;
    }

    int score = 0;

    if (total_speed <= 16) {
        score += 0;
    } else if (total_speed > 16 && total_speed <= 54) {
        score += 1;
    } else if (total_speed > 54 && total_speed <= 200) {
        score += 2;
    } else {
        score += 3;
    }

    if (total_ping >= 300) {
        score += 0;
    } else if (total_ping >= 120) {
        score += 1;
    } else {
        score += 2;
    }

    std::cout << std::endl;

    switch (score) {
        case 0:
            std::cout << colors.red_text("Вам стоит задуматься о смене провайдера. 0/5 баллов") << std::endl;
            break;
        case 1:
            std::cout << colors.red_text("У вас плохой интернет. 1/5 баллов") << std::endl;
            break;
        case 2:
            std::cout << colors.red_text("У вас плохой интернет. 2/5 баллов") << std::endl;
            break;
        case 3:
            std::cout << colors.green_text("У вас нормальный интернет. 3/5 баллов") << std::endl;
            break;
        case 4:
            std::cout << colors.green_text("У вас хороший интернет. 4/5 баллов") << std::endl;
            break;
        default:
            std::cout << colors.blue_text("У вас отличный интернет. 5/5 баллов") << std::endl;
            break;
    }

    std::cout << std::endl;

    std::cout << colors.blue_text("Нажмите Enter для выхода");
    std::cin.ignore();

    return 0;
}
