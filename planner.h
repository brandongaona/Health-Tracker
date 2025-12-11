// planner.h
#pragma once
#include <cmath>

enum class Sex { Male, Female };
enum class Units { Metric, Imperial };
enum class Activity { Sedentary=1, Light=2, Moderate=3, Very=4, Extra=5 };
enum class Goal { Cut, Maintain, Bulk };
enum class Pace { Slow, Normal, Aggressive };

struct UserInput {
    Sex sex;
    Units units;
    int ageYears;
    double height; // cm
    double weight; // kg
    Activity activity;
    Goal goal;
    Pace pace;
};

struct MacroPlan {
    double calories;
    double protein_g;
    double fat_g;
    double carbs_g;
};

struct PlanResult {
    double bmr;
    double tdee;
    double targetCalories;
    double weeklyChangeKg;
    double weeklyChangeLb;
    Pace   paceUsed;
    MacroPlan macros;
};

// implemented in planner.cpp (your refactored code)
PlanResult computePlan(const UserInput& u);
