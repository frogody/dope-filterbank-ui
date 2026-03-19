#include "LadderFilter.h"

void LadderFilter::prepare(double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;
    thermalCoeff = static_cast<float>(std::exp(-1.0 / (0.01 * sr))); // 10ms smoothing
    reset();
    updateCoefficients();
}

void LadderFilter::setCutoff(float freqHz)
{
    cutoff = std::clamp(freqHz, 20.f, static_cast<float>(sr * 0.49));
    updateCoefficients();
}

void LadderFilter::setResonance(float reso)
{
    resonance = std::clamp(reso, 0.f, 1.f);
    updateCoefficients();
}

void LadderFilter::setDrive(float d)
{
    drive = std::clamp(d, 0.f, 1.f);
    // Map 0..1 to saturation curve: 0 = no extra saturation, 1 = heavy per-stage
    saturationAmount = drive * drive; // quadratic curve for smoother control
}

void LadderFilter::reset()
{
    for (auto& ch : state)
        for (auto& s : ch.s)
            s = 0.0;
    thermalState[0] = thermalState[1] = 0.f;
}

void LadderFilter::updateCoefficients()
{
    // Simplified Moog ladder: one-pole coefficient
    g = 1.0 - std::exp(-2.0 * M_PI * cutoff / sr);
    // Resonance: 0..1 maps to 0..4.0 (allow self-oscillation at max)
    resonanceScaled = resonance * 4.0;
}

float LadderFilter::processSample(int channel, float input)
{
    auto& st = state[channel];

    // === Thermal drift: signal energy slightly modulates cutoff ===
    // This simulates analog component heating — louder signals slightly shift the filter
    float signalEnergy = std::abs(input);
    thermalState[channel] = thermalCoeff * thermalState[channel]
                          + (1.f - thermalCoeff) * signalEnergy;
    // Drift amount: up to ±2% cutoff shift based on drive
    double thermalDrift = 1.0 + (double)(thermalState[channel] * saturationAmount * 0.02);
    double gMod = 1.0 - std::exp(-2.0 * M_PI * cutoff * thermalDrift / sr);

    // === Feedback from 4th stage with nonlinear saturation ===
    double feedback = std::tanh(st.s[3] * resonanceScaled);

    // Input with feedback subtraction + saturation
    double u = std::tanh(input - feedback);

    // === Four cascaded one-pole stages with per-stage saturation ===
    for (int i = 0; i < 4; ++i)
    {
        st.s[i] += gMod * (u - st.s[i]);

        // Per-stage nonlinear saturation (the "fat" factor)
        // When drive=0: no extra saturation (clean Huovilainen)
        // When drive=1: heavy tanh saturation after each stage (Moog circuit overload)
        if (saturationAmount > 0.001f)
        {
            // Blend between clean and saturated based on drive
            double clean = st.s[i];
            double saturated = (double)fastTanh((float)(st.s[i] * (1.0 + saturationAmount * 3.0)));
            st.s[i] = clean * (1.0 - saturationAmount) + saturated * saturationAmount;
        }

        u = st.s[i];
    }

    switch (type)
    {
        case Type::LP: return static_cast<float>(st.s[3]);
        case Type::HP: return static_cast<float>(input - st.s[3]);
        case Type::BP: return static_cast<float>(st.s[1] - st.s[3]);
    }
    return static_cast<float>(st.s[3]);
}
