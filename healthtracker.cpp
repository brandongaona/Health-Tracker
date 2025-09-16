#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cmath>

using namespace std;

/*
 Assumptions / notes:
 - BMR: Mifflin–St Jeor
   Male:   BMR = 10*kg + 6.25*cm - 5*age + 5
   Female: BMR = 10*kg + 6.25*cm - 5*age - 161
 - Activity multipliers (TDEE = BMR * factor):
   1) Sedentary        1.20
   2) Lightly active   1.375
   3) Moderately active1.55
   4) Very active      1.725
   5) Extra active     1.90
 - Goal pace (weekly weight change):
   Slow:   ~0.25% body mass / week
   Normal: ~0.5% body mass / week
   Aggro:  ~1.0% body mass / week
   Converted to daily kcal via 1 lb ~ 3500 kcal (or 1 kg ~ 7700 kcal).
 - Macros:
   Protein: cut=2.2 g/kg, maintain=1.8 g/kg, bulk=1.6 g/kg
   Fat: 25% of calories (changeable)
   Carbs: remainder
 - Calorie per gram: protein=4, carbs=4, fat=9
 - This is an estimate for healthy adults, not medical advice.
*/

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
    // Mifflin–St Jeor
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
    // Choose default protein targets if not provided
    double ppk;
    if (protein_g_per_kg > 0) {
        ppk = protein_g_per_kg;
    } else {
        if (goal == Goal::Cut)      ppk = 2.2;
        else if (goal == Goal::Maintain) ppk = 1.8;
        else                        ppk = 1.6; // bulk
    }

    double protein_g = ppk * weightKg;
    double fat_kcal  = calories * fat_pct;
    double fat_g     = fat_kcal / 9.0;
    double protein_kcal = protein_g * 4.0;

    // Carbs take the remainder
    double remaining_kcal = max(0.0, calories - protein_kcal - fat_kcal);
    double carbs_g = remaining_kcal / 4.0;

    return { calories, protein_g, fat_g, carbs_g };
}

int main() {
    cout << fixed << setprecision(1);
    cout << "Daily Calorie & Macro Estimator\n"
            "--------------------------------\n";

    UserInput u;
    u.sex = getSex();
    u.units = getUnits();
    u.ageYears = getInt("Age (years, 14-100): ", 14, 100);

    if (u.units == Units::Metric) {
        u.weight = getDouble("Weight (kg, 35-300): ", 35.0, 300.0);
        u.height = getDouble("Height (cm, 130-230): ", 130.0, 230.0);
    } else {
        double lb  = getDouble("Weight (lb, 80-660): ", 80.0, 660.0);
        double in  = getDouble("Height (in, 50-90): ", 50.0, 90.0);
        u.weight = lb * 0.45359237;           // to kg
        u.height = in * 2.54;                 // to cm
    }

    u.activity = getActivity();
    u.goal     = getGoal();
    u.pace     = (u.goal == Goal::Maintain) ? Pace::Normal : getPace();

    // Compute BMR and TDEE
    double bmr  = bmrMifflinStJeor(u.sex, u.weight, u.height, u.ageYears);
    double tdee = bmr * activityFactor(u.activity);

    // Compute daily calorie adjustment from pace:
    // weekly weight change = paceFraction * body weight (kg or lb)
    // Convert to kcals/day via 1 kg ~ 7700 kcal.
    double weeklyChangeKg = paceFraction(u.pace) * u.weight;
    double weeklyKcal = weeklyChangeKg * 7700.0; // kcal/week
    double dailyKcalAdjust = weeklyKcal / 7.0;   // kcal/day

    double targetCalories = tdee;
    if (u.goal == Goal::Cut)      targetCalories = tdee - dailyKcalAdjust;
    else if (u.goal == Goal::Bulk) targetCalories = tdee + dailyKcalAdjust;

    // Sanity clamp
    if (u.goal == Goal::Cut) {
        // avoid overly low intakes; simplistic floor
        targetCalories = max(targetCalories, bmr * 1.1); // don't drop below ~10% over BMR
    }

    // Compute macros
    MacroPlan plan = computeMacros(u.goal, u.weight, targetCalories);

    // Output
    cout << "\n=== Results ===\n";
    cout << "BMR (Mifflin–St Jeor): " << bmr << " kcal/day\n";
    cout << "TDEE (activity-adjusted): " << tdee << " kcal/day\n";

    string goalStr = (u.goal == Goal::Cut ? "Cut (lose)" :
                     (u.goal == Goal::Bulk ? "Bulk (gain)" : "Maintain"));
    cout << "Goal: " << goalStr << "\n";

    if (u.goal != Goal::Maintain) {
        string paceStr = (u.pace == Pace::Slow ? "Slow (~0.25%/wk)" :
                         (u.pace == Pace::Normal ? "Normal (~0.5%/wk)" :
                                                   "Aggressive (~1.0%/wk)"));
        cout << "Pace: " << paceStr << "\n";
        cout << "Estimated weekly change: " << weeklyChangeKg << " kg/week ("
             << weeklyChangeKg * 2.20462 << " lb/week)\n";
    }

    cout << "\nTarget Calories: " << plan.calories << " kcal/day\n";
    cout << "Protein: " << plan.protein_g << " g/day\n";
    cout << "Fat:     " << plan.fat_g     << " g/day\n";
    cout << "Carbs:   " << plan.carbs_g   << " g/day\n";

    // Bonus: show same macros in percentages
    double pK = plan.protein_g * 4.0;
    double fK = plan.fat_g * 9.0;
    double cK = plan.carbs_g * 4.0;
    double total = max(1.0, pK + fK + cK);
    cout << setprecision(0);
    cout << "\nMacro Calorie Split (approx): "
         << round(pK / total * 100) << "% protein, "
         << round(fK / total * 100) << "% fat, "
         << round(cK / total * 100) << "% carbs\n";

    cout << setprecision(1);
    cout << "\nTips:\n"
            "- Adjust protein between 1.6–2.2 g/kg depending on goal and preference.\n"
            "- Fat can range ~20–35% of calories; carbs take the remainder.\n"
            "- Recalculate every ~4–6 weeks as weight/activity change.\n";

    return 0;
}
