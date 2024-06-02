#include "QuizManagerWidget.h"
#include "widgets/VocabularyQuizWidget.h"
#include "widgets/packages/SettingsDataPackage.h"

namespace tadaima
{
    namespace gui
    {
        namespace quiz
        {
            QuizManagerWidget::QuizManagerWidget(tools::Logger& logger) :m_logger(logger), quizWidgetOpen(false)
            {

            }

            void QuizManagerWidget::startQuiz(QuizType type, const std::vector<Lesson>& lesson)
            {
                if( QuizType::MultipleChoiceQuiz == type )
                {
                    m_quiz.reset();
                    m_logger.log("Starting MultipleChoiceQuiz.", tools::LogLevel::INFO);
                    m_quiz = std::make_unique<widget::QuizWidget>(m_logger, lesson);
                    quizWidgetOpen = true;
                }
                else if( QuizType::VocabularyQuiz == type )
                {
                    m_quiz.reset();
                    m_logger.log("Starting VocabularyQuiz.", tools::LogLevel::INFO);
                    m_quiz = std::make_unique<widget::VocabularyQuizWidget>(m_askedWordType, m_answerWordType , lesson, m_logger );
                    quizWidgetOpen = true;
                }
            }

            void QuizManagerWidget::draw([[maybe_unused]] bool* p_open)
            {
                if( quizWidgetOpen && m_quiz )
                {
                    m_quiz->draw(&quizWidgetOpen);
                }
            }

            void QuizManagerWidget::initialize(const tools::DataPackage& r_package)
            {
                try
                {
                    const widget::SettingsDataPackage* package = dynamic_cast<const widget::SettingsDataPackage*>(&r_package);
                    if( package )
                    {
                        m_logger.log("QuizManager::initialize.", tools::LogLevel::INFO);

                        m_answerWordType = package->get< quiz::WordType>(widget::SettingsPackageKey::AnswerWordType);
                        m_askedWordType = package->get<quiz::WordType>(widget::SettingsPackageKey::AskedWordType);
                    }
                }
                catch( std::exception& exception )
                {
                    m_logger.log("Exception caught in QuizManager::initialize: " + std::string(exception.what()), tools::LogLevel::PROBLEM);
                }
                catch( ... )
                {
                    m_logger.log("Exception caught in QuizManager::initialize.Unkown problem.", tools::LogLevel::PROBLEM);
                }
            }

        }
    }
}