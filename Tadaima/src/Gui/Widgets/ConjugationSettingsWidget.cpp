#include "ConjugationSettingsWidget.h"
#include "imgui.h"
#include "Tools/Logger.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {

            ConjugationSettingsWidget::ConjugationSettingsWidget(tadaima::Dictionary& r_Dictionary, tools::Logger& logger, bool& conjugationJustSaved) :
                mr_Dictionary(r_Dictionary),
                m_logger(logger),
                m_conjugationJustSaved(conjugationJustSaved)
            {

            }

            void ConjugationSettingsWidget::start()
            {
                ImGui::OpenPopup("Conjugations Modal");
            }

            void ConjugationSettingsWidget::initialize(tadaima::Word& r_WordToEdit)
            {
                mp_Word = &r_WordToEdit;

                if( mp_Word->kana.size() < sizeof(m_conjugationWord) )
                {
                    memset(m_conjugationWord, 0, sizeof(m_conjugationWord));
                    memcpy(m_conjugationWord, mp_Word->kana.c_str(), mp_Word->kana.size());
                }

                for( size_t index = 0; index < CONJUGATION_COUNT; ++index )
                {
                    memset(&m_conjugationBuffers[index], 0, m_conjugationBuffers[index].size());
                    std::strncpy(m_conjugationBuffers[index].data(), mp_Word->conjugations[index].c_str(), mp_Word->conjugations[index].size());
                }
            }

            void ConjugationSettingsWidget::draw()
            {
                ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));

                // Disable scrolling for the popup window
                if( ImGui::BeginPopupModal("Conjugations Modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) )
                {
                    std::string title = "Conjugations for " + std::string(m_conjugationWord);
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), title.c_str());

                    ImGui::InputText("##ConjugationInput", m_conjugationWord, sizeof(m_conjugationWord));

                    ImGui::Separator();
                    ImGui::Spacing();

                    // Autofill button
                    if( ImGui::Button("Autofill Conjugations") )
                    {
                        m_logger.log("Autofilling conjugations for word: " + std::string(m_conjugationWord), tools::LogLevel::INFO);

                        try
                        {
                            auto conjugations = mr_Dictionary.getConjugations(std::string(mp_Word->romaji));

                            // Update buffers with fetched conjugations
                            for( int i = 0; i < CONJUGATION_COUNT; ++i )
                            {
                                std::strncpy(m_conjugationBuffers[i].data(), conjugations[i].c_str(), m_conjugationBuffers[i].size() - 1);
                                m_conjugationBuffers[i][m_conjugationBuffers[i].size() - 1] = '\0';
                            }

                            m_logger.log("Conjugations autofilled successfully.", tools::LogLevel::INFO);
                        }
                        catch( const std::exception& e )
                        {
                            m_logger.log(std::string("Error while autofilling conjugations: ") + e.what(), tools::LogLevel::PROBLEM);
                        }
                    }

                    ImGui::Spacing();

                    // Begin a child region for the scrollable table
                    ImGui::BeginChild("ConjugationsTableRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), true, ImGuiWindowFlags_HorizontalScrollbar);

                    if( ImGui::BeginTable("ConjugationsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable) )
                    {
                        ImGui::TableSetupColumn("Conjugation Type", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("Conjugated Form", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableHeadersRow();

                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            std::string name = tadaima::ConjugationTypeToString(static_cast<tadaima::ConjugationType>(i));
                            ImGui::Text(name.c_str());

                            ImGui::TableNextColumn();
                            ImGui::PushID(i);
                            ImGui::InputText("##ConjugationInput", m_conjugationBuffers[i].data(), sizeof(m_conjugationBuffers[i]));
                            ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }

                    ImGui::EndChild(); // End the scrollable child region

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Keep the Save and Cancel buttons at the bottom
                    if( ImGui::Button("Save", ImVec2(100, 0)) )
                    {
                        m_logger.log("Saving conjugations for current word", tools::LogLevel::INFO);
                        for( int i = 0; i < CONJUGATION_COUNT; ++i )
                        {
                            mp_Word->conjugations[i] = std::string(m_conjugationBuffers[i].data());
                        }

                        m_conjugationJustSaved = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if( ImGui::Button("Cancel", ImVec2(100, 0)) )
                    {
                        m_logger.log("Cancelling conjugations modal", tools::LogLevel::INFO);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopStyleVar();
            }

            void ConjugationSettingsWidget::clear()
            {
                memset(m_conjugationWord, 0, sizeof(m_conjugationWord));
                clearConjugationBuffers(m_conjugationBuffers);
            }

            void ConjugationSettingsWidget::clearConjugationBuffers(std::array<std::array<char, 128>, CONJUGATION_COUNT>& buffers)
            {
                for( auto& innerArray : buffers )
                {
                    for( char& c : innerArray )
                    {
                        c = '\0'; // Reset each character to null terminator
                    }
                }
            }
        }
    }
}
