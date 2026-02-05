#include "food_api.h"
#include "json.hpp"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include "config.h"

using json = nlohmann::json;
using namespace std;

// Callback function for cURL to write response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Make HTTP GET request
string httpGet(const string& url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // For Windows SSL issues
        
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            cerr << "cURL error: " << curl_easy_strerror(res) << endl;
        }
        
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Search for foods in USDA database
vector<FoodItem> searchFoods(const string& query, int maxResults) {
    vector<FoodItem> results;
    
    // URL encode the query
    CURL* curl = curl_easy_init();
    char* encoded = curl_easy_escape(curl, query.c_str(), query.length());
    string encodedQuery(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    // Build API URL
    string url = "https://api.nal.usda.gov/fdc/v1/foods/search?query=" + encodedQuery 
                 + "&pageSize=" + to_string(maxResults)
                 + "&api_key=" + USDA_API_KEY;
    
    // Make request
    string response = httpGet(url);
    
    try {
        auto jsonResponse = json::parse(response);
        
        if (jsonResponse.contains("foods")) {
            for (const auto& food : jsonResponse["foods"]) {
                FoodItem item;
                item.fdcId = food.value("fdcId", 0);
                item.description = food.value("description", "Unknown");
                
                // Initialize to 0
                item.calories = 0;
                item.protein_g = 0;
                item.carbs_g = 0;
                item.fat_g = 0;
                
                // Extract nutrients
                if (food.contains("foodNutrients")) {
                    for (const auto& nutrient : food["foodNutrients"]) {
                        string name = nutrient.value("nutrientName", "");
                        double value = nutrient.value("value", 0.0);
                        
                        if (name == "Energy" || name.find("Energy") != string::npos) {
                            item.calories = value;
                        } else if (name == "Protein") {
                            item.protein_g = value;
                        } else if (name.find("Carbohydrate") != string::npos) {
                            item.carbs_g = value;
                        } else if (name.find("Total lipid") != string::npos || name == "Fat") {
                            item.fat_g = value;
                        }
                    }
                }
                
                results.push_back(item);
                
                if (results.size() >= maxResults) break;
            }
        }
    } catch (const exception& e) {
        cerr << "JSON parsing error: " << e.what() << endl;
    }
    
    return results;
}

// Recommend foods based on goals
FoodRecommendations recommendFoods(const string& goal, double targetProtein, double targetCalories) {
    FoodRecommendations recommendations;
    recommendations.goal_type = goal;
    
    vector<string> searchTerms;
    
    if (goal == "cut") {
        // High protein, low calorie foods
        searchTerms = {"chicken breast", "egg whites", "greek yogurt", "tilapia", "cod fish"};
    } else if (goal == "bulk") {
        // Calorie-dense, muscle-building foods
        searchTerms = {"peanut butter", "whole milk", "salmon", "pasta", "oats"};
    } else { // maintain
        // Balanced foods
        searchTerms = {"brown rice", "chicken", "broccoli", "sweet potato", "almonds"};
    }
    
    // Search for each term and take the best result
    for (const auto& term : searchTerms) {
        auto foods = searchFoods(term, 1);
        if (!foods.empty()) {
            recommendations.foods.push_back(foods[0]);
        }
    }
    
    return recommendations;
}