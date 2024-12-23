#include "Conjugations.h"
#include <format>

namespace tadaima
{
    std::string ConjugationTypeToString(ConjugationType type)
    {
        switch( type )
        {
            case PLAIN: return "Plain";
            case POLITE: return "Polite";
            case TE_FORM: return "Te Form";
            case PAST: return "Past Form";
            case POLITE_PAST: return "Past Positive Formal";
            case NEGATIVE: return "Negative";
            case PAST_NEGATIVE: return "Past Negative";
            case POLITE_PAST_NEGATIVE: return "Polite Past Negative";
            case POLITE_NEGATIVE: return "Polite Negative";
            case POTENTIAL: return "Potential";
            case PASSIVE: return "Passive";
            case CAUSATIVE: return "Causative";
            case CONDITIONAL: return "Conditional";
            case VOLITIONAL: return "Volitional";
            case IMPERATIVE: return "Imperative";

            default: return "Unknown";
        }
    }

}