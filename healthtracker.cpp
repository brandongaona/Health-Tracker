#include "planner.h" // Include the header so we know what UserInput/PlanResult are
#include <iostream>
#include <cmath>
#include <algorithm> // Needed for max()

using namespace std;

// --- Logic Helpers ---

double activityFactor(Activity a) {
    switch (a) {
        case Activity::Sedentary: return 1.20;
        case Activity::Light:     return 1.375;
        case Activity::Moderate:  return 1.55;
        case Activity::Very:      return 1.725;
        case Activity::Extra:     return 1.90;
        default: return 1.55;
    }
}

double bmrMifflinStJeor(Sex sex, double kg, double cm, int age) {
    double base = 10.0 * kg + 6.25 * cm - 5.0 * age;
    if (sex == Sex::Male)  return base + 5.0;
    else                   return base - 161.0;
}

double paceFraction(Pace p) {
    switch (p) {
        case Pace::Slow:       return 0.0025; // 0.25%
        case Pace::Normal:     return 0.0050; // 0.5%
        case Pace::Aggressive: return 0.0100; // 1.0%
        default: return 0.0050;
    }
}

MacroPlan computeMacros(Goal goal, double weightKg, double calories,
                        double protein_g_per_kg = -1.0, double fat_pct = 0.25) {
    double ppk;
    if (protein_g_per_kg > 0) {
        ppk = protein_g_per_kg;
    } else {
        if (goal == Goal::Cut)           ppk = 2.2;
        else if (goal == Goal::Maintain) ppk = 1.8;
        else                             ppk = 1.6; // bulk
    }

    double protein_g = ppk * weightKg;
    double fat_kcal  = calories * fat_pct;
    double fat_g     = fat_kcal / 9.0;
    double protein_kcal = protein_g * 4.0;

    double remaining_kcal = max(0.0, calories - protein_kcal - fat_kcal);
    double carbs_g = remaining_kcal / 4.0;

    return { calories, protein_g, fat_g, carbs_g };
}

// --- Main Calculation Function ---

PlanResult computePlan(const UserInput& u) {
    PlanResult res;

    res.bmr  = bmrMifflinStJeor(u.sex, u.weight, u.height, u.ageYears);
    res.tdee = res.bmr * activityFactor(u.activity);

    Pace pace = (u.goal == Goal::Maintain) ? Pace::Normal : u.pace;
    res.paceUsed = pace;

    double weeklyChangeKg = paceFraction(pace) * u.weight;
    res.weeklyChangeKg = weeklyChangeKg;
    res.weeklyChangeLb = weeklyChangeKg * 2.20462;

    double weeklyKcal = weeklyChangeKg * 7700.0;
    double dailyKcalAdjust = weeklyKcal / 7.0;

    double targetCalories = res.tdee;
    if (u.goal == Goal::Cut)       targetCalories = res.tdee - dailyKcalAdjust;
    else if (u.goal == Goal::Bulk) targetCalories = res.tdee + dailyKcalAdjust;

    if (u.goal == Goal::Cut) {
        targetCalories = max(targetCalories, res.bmr * 1.1);
    }

    res.targetCalories = targetCalories;
    res.macros = computeMacros(u.goal, u.weight, targetCalories);

    return res;
}