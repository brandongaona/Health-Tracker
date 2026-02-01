#include <iostream>
#include <fstream>   // NEW: Needed to read html files
#include <streambuf> // NEW: Needed to read html files

#include "httplib.h"
#include "json.hpp"
#include "planner.h"

using json = nlohmann::json;
using namespace std;
using namespace httplib;

// --- small helpers to map strings <-> enums ---

Sex parseSex(const string& s) {
    if (s == "male" || s == "Male" || s == "M") return Sex::Male;
    return Sex::Female;
}

Activity parseActivity(const string& s) {
    if (s == "sedentary")  return Activity::Sedentary;
    if (s == "light")      return Activity::Light;
    if (s == "moderate")   return Activity::Moderate;
    if (s == "very")       return Activity::Very;
    if (s == "extra")      return Activity::Extra;
    return Activity::Moderate;
}

Goal parseGoal(const string& s) {
    if (s == "cut")      return Goal::Cut;
    if (s == "bulk")     return Goal::Bulk;
    return Goal::Maintain;
}

Pace parsePace(const string& s) {
    if (s == "slow")       return Pace::Slow;
    if (s == "aggressive") return Pace::Aggressive;
    return Pace::Normal;
}

// --- CORS helper ---
void add_cors_headers(Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS, GET");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    Server svr;

    // --- 1. SERVE STATIC FILES (HTML/CSS) ---
    
    // Serve any file from the current directory (like styles.css, login.html)
    svr.set_mount_point("/", "./");

    // Explicitly handle the homepage ("/") to show index.html
    svr.Get("/", [](const Request& req, Response& res) {
        std::ifstream file("login.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("login.html not found", "text/plain");
        }
    });
    svr.Get("/index", [](const Request& req, Response& res) {
        std::ifstream file("index.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
        }
    });
    

    // --- 2. API ENDPOINTS ---
    // Handle CORS preflight
    svr.Options(R"(.*)", [](const Request& req, Response& res) {
        add_cors_headers(res);
    });

    svr.Post("/plan", [](const Request& req, Response& res) {
        add_cors_headers(res);
        try {
            auto body = json::parse(req.body);
            UserInput u;
            u.units    = Units::Metric; 
            u.sex      = parseSex(body.at("sex").get<string>());
            u.ageYears = body.at("age").get<int>();
            u.height   = body.at("height_cm").get<double>();
            u.weight   = body.at("weight_kg").get<double>();
            u.activity = parseActivity(body.at("activity").get<string>());
            u.goal     = parseGoal(body.at("goal").get<string>());
            u.pace     = parsePace(body.at("pace").get<string>());

            PlanResult r = computePlan(u);

            json out;
            out["bmr"]            = r.bmr;
            out["tdee"]           = r.tdee;
            out["targetCalories"] = r.targetCalories;
            out["weeklyChangeKg"] = r.weeklyChangeKg;
            out["weeklyChangeLb"] = r.weeklyChangeLb;

            out["macros"] = {
                {"calories",  r.macros.calories},
                {"protein_g", r.macros.protein_g},
                {"fat_g",     r.macros.fat_g},
                {"carbs_g",   r.macros.carbs_g}
            };

            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            json err;
            err["error"] = string("Bad request: ") + e.what();
            res.set_content(err.dump(), "application/json");
        }
    });

    cout << "Listening on http://0.0.0.0:8080\n";
    svr.listen("0.0.0.0", 8080);
}