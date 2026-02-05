#ifndef FOOD_API_H
#define FOOD_API_H

#include <string>
#include <vector>

struct FoodItem {
    int fdcId;
    std::string description;
    double calories;
    double protein_g;
    double carbs_g;
    double fat_g;
};

struct FoodRecommendations {
    std::vector<FoodItem> foods;
    std::string goal_type; // "cut", "bulk", or "maintain"
};

// Search USDA database
std::vector<FoodItem> searchFoods(const std::string& query, int maxResults = 5);

// Get food recommendations based on goals
FoodRecommendations recommendFoods(const std::string& goal, double targetProtein, double targetCalories);

#endif