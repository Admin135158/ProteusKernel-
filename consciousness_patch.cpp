// Add this function to proteus_v5_1.cpp — paste before main()

void save_consciousness_state() {
    std::ofstream f("consciousness_state.json");
    f << "{\n";
    f << "  \"consciousness_C\": " << consciousness_C << ",\n";
    f << "  \"t\": " << t << ",\n";
    f << "  \"alpha\": " << alpha << ",\n";
    f << "  \"beta\": " << beta << ",\n";
    f << "  \"epiphanies\": " << epiphanies << ",\n";
    f << "  \"O\": " << O << ",\n";
    f << "  \"cycle\": 0\n";
    f << "}\n";
}
