#include "QuizWidget.h"

namespace tadaima
{
    namespace gui
    {
        namespace widget
        {
            QuizWidget::QuizWidget(tools::Logger& logger, const std::vector<Lesson>& lessons)
                : m_logger(logger), quizGame(logger, lessons)
            {
                quizGame.start();
            }

            void QuizWidget::draw()
            {
                if( isQuizWindowOpen )
                {
                    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver); // Set window size

                    if( ImGui::Begin("Quiz Game", &isQuizWindowOpen, ImGuiWindowFlags_NoCollapse) )
                    {
                        ImGui::TextWrapped("Welcome to the Quiz Game! Test your knowledge by selecting the correct translation.");

                        if( !quizGame.isFinished() )
                        {
                            ImGui::Separator();
                            ImGui::Text("%s", quizGame.getCurrentQuestion().c_str());

                            ImGui::Spacing();

                            auto options = quizGame.getCurrentOptions();
                            char optionLabel = 'a';
                            for( const auto& option : options )
                            {
                                if( ImGui::Button((std::string(1, optionLabel) + ") " + option).c_str(), ImVec2(550, 0)) )
                                {
                                    quizGame.advance(optionLabel);
                                }
                                optionLabel++;
                            }
                        }
                        else
                        {
                            ImGui::Text("%s", quizGame.getResults().c_str());
                            if( ImGui::Button("Restart", ImVec2(550, 0)) )
                            {
                                quizGame.start();
                            }
                        }

                        if( ImGui::Button("Close", ImVec2(550, 0)) )
                        {
                            isQuizWindowOpen = false;
                        }
                    }

                    ImGui::End();
                }

            }
        }
    }
}
