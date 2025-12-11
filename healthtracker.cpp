#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cmath>

using namespace std;

enum class Sex { Male, Female };
enum class Units { Metric, Imperial };
enum class Activity { Sedentary=1, Light=2, Moderate=3, Very=4, Extra=5 };
enum class Goal { Cut, Maintain, Bulk };
enum class Pace { Slow, Normal, Aggressive };

struct UserInput {
    Sex sex;
    Units units;
    int ageYears;
    double height; // cm (internal)
    double weight; // kg (internal)
    Activity activity;
    Goal goal;
    Pace pace;
};

double getDouble(const string& prompt, double minVal, double maxVal) {
    while (true) {
        cout << prompt;
        double x;
        if (cin >> x && x >= minVal && x <= maxVal) return x;
        cout << "Invalid input. Please enter a number in [" << minVal << ", " << maxVal << "].\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int getInt(const string& prompt, int minVal, int maxVal) {
    while (true) {
        cout << prompt;
        int x;
        if (cin >> x && x >= minVal && x <= maxVal) return x;
        cout << "Invalid input. Please enter an integer in [" << minVal << ", " << maxVal << "].\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

Sex getSex() {
    while (true) {
        cout << "Sex (1 = Male, 2 = Female): ";
        int s;
        if (cin >> s && (s == 1 || s == 2)) return s == 1 ? Sex::Male : Sex::Female;
        cout << "Please enter 1 or 2.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

Units getUnits() {
    while (true) {
        cout << "Units (1 = Metric kg/cm, 2 = Imperial lb/in): ";
        int u;
        if (cin >> u && (u == 1 || u == 2)) return u == 1 ? Units::Metric : Units::Imperial;
        cout << "Please enter 1 or 2.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

Activity getActivity() {
    cout << "Activity level:\n"
            " 1) Sedentary (little/no exercise)\n"
            " 2) Lightly active (1-3 days/wk)\n"
            " 3) Moderately active (3-5 days/wk)\n"
            " 4) Very active (6-7 days/wk)\n"
            " 5) Extra active (physical job + training)\n";
    int a = getInt("Choose 1-5: ", 1, 5);
    return static_cast<Activity>(a);
}

Goal getGoal() {
    cout << "Goal:\n"
            " 1) Cut (lose fat)\n"
            " 2) Maintain\n"
            " 3) Bulk (gain muscle)\n";
    int g = getInt("Choose 1-3: ", 1, 3);
    if (g == 1) return Goal::Cut;
    if (g == 2) return Goal::Maintain;
    return Goal::Bulk;
}

Pace getPace() {
    cout << "Pace (weekly rate):\n"
            " 1) Slow (~0.25% body weight / week)\n"
            " 2) Normal (~0.5% body weight / week)\n"
            " 3) Aggressive (~1.0% body weight / week)\n";
    int p = getInt("Choose 1-3: ", 1, 3);
    if (p == 1) return Pace::Slow;
    if (p == 2) return Pace::Normal;
    return Pace::Aggressive;
}

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

struct MacroPlan {
    double calories;
    double protein_g;
    double fat_g;
    double carbs_g;
};

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

// -------- NEW: core result struct + pure computePlan --------

struct PlanResult {
    double bmr;
    double tdee;
    double targetCalories;
    double weeklyChangeKg;
    double weeklyChangeLb;
    Pace   paceUsed;
    MacroPlan macros;
};

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