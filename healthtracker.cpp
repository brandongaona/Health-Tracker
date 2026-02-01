#include "planner.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace {
    const double KCAL_PER_G_FAT      = 9.0;
    const double KCAL_PER_G_PROTEIN  = 4.0;
    const double KCAL_PER_G_CARB     = 4.0;
    const double KCAL_PER_KG_FAT     = 7700.0;  // Approximation for body fat loss/gain
    const double MIN_CALORIE_MULTIPLIER = 1.1;   // Minimum calories as % of BMR
    const double EPSILON             = 1e-9;     // For floating-point comparisons
}

void validateInput(const UserInput& u) {
    if (u.weight <= 0 || u.weight > 500)
        throw invalid_argument("Weight must be between 0 and 500 kg");
    if (u.height <= 0 || u.height > 300)
        throw invalid_argument("Height must be between 0 and 300 cm");
    if (u.ageYears < 15 || u.ageYears > 120)
        throw invalid_argument("Age must be between 15 and 120 years");
}

double activityFactor(Activity a) {
    switch (a) {
        case Activity::Sedentary: return 1.20;
        case Activity::Light:     return 1.375;
        case Activity::Moderate:  return 1.55;
        case Activity::Very:      return 1.725;
        case Activity::Extra:     return 1.90;
        default:                  return 1.55;
    }
}

double bmrMifflinStJeor(Sex sex, double kg, double cm, int age) {
    double base = 10.0 * kg + 6.25 * cm - 5.0 * age;
    return (sex == Sex::Male) ? base + 5.0 : base - 161.0;
}

double paceFraction(Pace p) {
    switch (p) {
        case Pace::Slow:       return 0.0025;  // 0.25%
        case Pace::Normal:     return 0.0050;  // 0.5%
        case Pace::Aggressive: return 0.0100;  // 1.0%
        default:               return 0.0050;
    }
}

MacroPlan computeMacros(Goal goal, double weightKg, double calories) {
    const double fatPercentage = 0.25;

    double proteinPerKg;
    switch (goal) {
        case Goal::Cut:      proteinPerKg = 2.2; break;
        case Goal::Maintain: proteinPerKg = 1.8; break;
        case Goal::Bulk:     proteinPerKg = 1.6; break;
        default:             proteinPerKg = 1.8;
    }

    double protein_g   = proteinPerKg * weightKg;
    double fat_kcal    = calories * fatPercentage;
    double fat_g       = fat_kcal / KCAL_PER_G_FAT;
    double protein_kcal = protein_g * KCAL_PER_G_PROTEIN;

    double remaining_kcal = max(0.0, calories - protein_kcal - fat_kcal);
    double carbs_g       = remaining_kcal / KCAL_PER_G_CARB;

    return { calories, protein_g, fat_g, carbs_g };
}

PlanResult computePlan(const UserInput& u) {
    validateInput(u);

    PlanResult res;

    // Calculate base metabolic rates
    res.bmr  = bmrMifflinStJeor(u.sex, u.weight, u.height, u.ageYears);
    res.tdee = res.bmr * activityFactor(u.activity);

    double targetCalories;

    if (u.goal == Goal::Maintain) {
        // Handle maintenance goal
        res.paceUsed      = Pace::Normal;
        res.weeklyChangeKg = 0.0;
        res.weeklyChangeLb = 0.0;
        targetCalories    = res.tdee;
    } else {
        // Calculate weight change targets (Cut or Bulk)
        res.paceUsed       = u.pace;
        double weeklyChangeKg = paceFraction(u.pace) * u.weight;
        res.weeklyChangeKg = weeklyChangeKg;
        res.weeklyChangeLb = weeklyChangeKg * 2.20462;

        double weeklyKcal     = weeklyChangeKg * KCAL_PER_KG_FAT;
        double dailyKcalAdjust = weeklyKcal / 7.0;

        // Calculate calorie targets
        targetCalories = (u.goal == Goal::Cut)
            ? res.tdee - dailyKcalAdjust
            : res.tdee + dailyKcalAdjust;

        // Safety floor when cutting: don't go below MIN_CALORIE_MULTIPLIER * BMR
        if (u.goal == Goal::Cut) {
            targetCalories = max(targetCalories, res.bmr * MIN_CALORIE_MULTIPLIER);
        }
    }

    res.targetCalories = targetCalories;
    res.macros         = computeMacros(u.goal, u.weight, targetCalories);

    return res;
}
