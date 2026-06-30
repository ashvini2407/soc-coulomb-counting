#include <stdio.h>

/* ---------------- Configuration ---------------- */
#define Q_RATED_AH      10.0f      // Battery rated capacity in Ah
#define SAMPLE_TIME_S   1.0f       // Sampling interval in seconds
#define SOC_MAX         100.0f
#define SOC_MIN         0.0f

/* ---------------- Global Variables ---------------- */
float soc = 50.0f;        // Initial SoC (%)
float current_amp = 0.0f; // Measured current (A) - positive=charging, negative=discharging
float voltage = 0.0f;     // Battery terminal voltage (for OCV correction)
float total_charge_Ah = 0.0f;

/* ---------------- Function Prototypes ---------------- */
float read_current_sensor(void);
float read_voltage_sensor(void);
float coulomb_counting_update(float soc_prev, float current, float dt);
float ocv_lookup(float v);
int   is_battery_resting(float current);

/* ---------------- Simulated Current Sensor ---------------- */
float read_current_sensor(void)
{
    // Real hardware-la: ADC value read panni current convert pannanum
    // Idhu simulation purpose
    return 2.0f;   // example: 2A charging current
}

/* ---------------- Simulated Voltage Sensor ---------------- */
float read_voltage_sensor(void)
{
    // Real hardware-la: ADC voltage read pannanum
    return 3.85f;  // example voltage
}

/* ---------------- Coulomb Counting Core Function ---------------- */
float coulomb_counting_update(float soc_prev, float current, float dt)
{
    float delta_Q_As;       // Amp-seconds
    float delta_soc;
    float soc_new;

    delta_Q_As = current * dt;                 // ΔQ = I × Δt
    total_charge_Ah += delta_Q_As / 3600.0f;    // convert As -> Ah, accumulate

    delta_soc = (delta_Q_As / (Q_RATED_AH * 3600.0f)) * 100.0f;
    soc_new = soc_prev + delta_soc;

    /* Clamp SoC between 0 - 100 */
    if (soc_new > SOC_MAX) soc_new = SOC_MAX;
    if (soc_new < SOC_MIN) soc_new = SOC_MIN;

    return soc_new;
}

/* ---------------- OCV Lookup Table (Simple Linear) ---------------- */
float ocv_lookup(float v)
{
    /* Simple linear approx between known points
       4.2V -> 100%, 3.0V -> 0% */
    float soc_percent;

    if (v >= 4.2f) return 100.0f;
    if (v <= 3.0f) return 0.0f;

    soc_percent = (v - 3.0f) / (4.2f - 3.0f) * 100.0f;
    return soc_percent;
}

/* ---------------- Check Battery Resting State ---------------- */
int is_battery_resting(float current)
{
    if (current > -0.05f && current < 0.05f)   // near zero current
        return 1;
    return 0;
}

/* ---------------- Main Loop (Simulated Embedded Loop) ---------------- */
int main(void)
{
    int sample;

    printf("Time(s)\tCurrent(A)\tVoltage(V)\tSoC(%%)\n");

    for (sample = 0; sample < 10; sample++)
    {
        current_amp = read_current_sensor();
        voltage     = read_voltage_sensor();

        if (is_battery_resting(current_amp))
        {
            /* Correction using OCV when battery is idle */
            soc = ocv_lookup(voltage);
        }
        else
        {
            /* Normal coulomb counting update */
            soc = coulomb_counting_update(soc, current_amp, SAMPLE_TIME_S);
        }

        printf("%d\t%.2f\t\t%.2f\t\t%.2f\n",
                sample, current_amp, voltage, soc);
    }

    printf("\nTotal Charge Added: %.4f Ah\n", total_charge_Ah);
    printf("Final SoC: %.2f%%\n", soc);

    return 0;
