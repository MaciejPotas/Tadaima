#include "QuizManager.h"


namespace tadaima
{
    namespace gui
    {
        QuizManager::QuizManager(tools::Logger& logger) :m_logger(logger), quizWidgetOpen(false)
        {
        }

        void QuizManager::startQuiz(const std::vector<Lesson>& lesson)
        {
            quizWidget = std::make_unique<widget::QuizWidget>(m_logger, lesson);
            quizWidgetOpen = true;
        }

        void QuizManager::draw()
        {
            if( quizWidgetOpen && quizWidget )
            {
                quizWidget->draw();
            }
        }
    }
}
