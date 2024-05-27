#include "QuizManager.h"
#include "widgets/VocabularyQuizWidget.h"



namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            QuizManager::QuizManager(tools::Logger& logger) :m_logger(logger), quizWidgetOpen(false)
            {

            }

            void QuizManager::startQuiz(QuizType type, const std::vector<Lesson>& lesson)
            {
                if( QuizType::MultipleChoiceQuiz == type )
                {
                    m_quiz = std::make_unique<widget::QuizWidget>(m_logger, lesson);
                    quizWidgetOpen = true;
                }
                else if( QuizType::VocabularyQuiz == type )
                {
                    m_quiz = std::make_unique<widget::VocabularyQuizWidget>(m_logger, lesson);
                    quizWidgetOpen = true;
                }
            }

            void QuizManager::draw()
            {
                if( quizWidgetOpen && m_quiz )
                {
                    m_quiz->draw(&quizWidgetOpen);
                }
            }
        }
    }
}