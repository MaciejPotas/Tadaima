#include "MainDashboardWidget.h"
#include "imgui.h"
#include "gui.h"
#include "packages/SettingsDataPackage.h"
#include <random>
#include <format>

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            // Utility function to get a random word of the day
            std::pair<std::string, std::string> getRandomWordOfTheDay()
            {
                std::vector<std::pair<std::string, std::string>> words = {
                    {"茶", "Tea (ちゃ)"},
                    {"本", "Book (ほん)"},
                    {"猫", "Cat (ねこ)"},
                    {"犬", "Dog (いぬ)"},
                    {"花", "Flower (はな)"}
                };

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, words.size() - 1);

                return words[dist(gen)];
            }

            void MainDashboardWidget::initialize(const tools::DataPackage& r_package)
            {
                const SettingsDataPackage* package = dynamic_cast<const SettingsDataPackage*>(&r_package);
                if( package )
                {
                    m_username = package->get<std::string>(SettingsPackageKey::Username);
                }

                // Initialize dynamic content
                std::tie(m_wordOfTheDay, m_wordMeaning) = getRandomWordOfTheDay();
                m_progress = 0.65f; // Example progress value, this should be dynamic in a real application
            }

            void MainDashboardWidget::draw(bool* p_open)
            {
                if( !ImGui::Begin("Main Dashboard", p_open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize) )
                {
                    ImGui::End();
                    return;
                }

                // Header
                ImGui::Text((const char*)std::format("Ohayou, {}!", m_username).c_str());
                ImGui::Text((const char*)u8"I am so lucky to see you again here :-)");
                ImGui::Text((const char*)u8"Let's learn some new words together!");

                // Progress
                ImGui::Separator();
                ImGui::Text("Your Learning Progress:");
                ImGui::ProgressBar(m_progress, ImVec2(-1.0f, 0.0f), std::format("{}% complete", static_cast<int>(m_progress * 100)).c_str());

                // Word of the Day
                ImGui::Separator();
                ImGui::Text((const char*)u8"Word of the Day:");
                ImGui::Text(std::format("{} - {}", m_wordOfTheDay, m_wordMeaning).c_str());

                // Performance Graphs (Example with placeholder values)
                ImGui::Separator();
                ImGui::Text("Performance Graphs:");
                ImGui::PlotLines("Vocab Acquisition", m_vocabPerformance.data(), m_vocabPerformance.size(), 0, nullptr, 0.0f, 1.0f, ImVec2(0, 80));
                ImGui::PlotLines("Pronunciation", m_pronunciationPerformance.data(), m_pronunciationPerformance.size(), 0, nullptr, 0.0f, 1.0f, ImVec2(0, 80));

                // Cultural Insight
                ImGui::Separator();
                ImGui::Text("Cultural Insight:");
                ImGui::TextWrapped("Golden Week - A week of holidays in Japan, which includes multiple national holidays in late April and early May.");

                ImGui::End();
            }
        }
    }
}
