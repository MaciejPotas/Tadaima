// -----------------------------------------------------------------------------
// File:        DataPackage.h
// Description: Header file for the DataPackage class.
//
//              DataPackage is a base class that is used by various GUI components
//              to pass data.
// -----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <variant>
#include <stdexcept>

namespace tools
{
    /**
    * @brief DataPackage is a base class for passing data between GUI components.
    *
    * This class serves as a base class for passing data between various GUI components.
    * Each child class of DataPackage should provide its own unique identifier
    * to distinguish its type.
    */
    class DataPackage
    {
    public:
        using Identifier = uint32_t; ///< Alias for the identifier.

        /**
        * @brief Constructor.
        *
        * Initializes the DataPackage with the provided identifier.
        *
        * @param id The identifier for the DataPackage.
        */
        DataPackage(Identifier id) : m_identifier(id) {}

        /**
        * @brief Virtual destructor.
        */
        virtual ~DataPackage() = default;

        /**
        * @brief Get the unique identifier of the DataPackage.
        *
        * @return The unique identifier of the DataPackage.
        */
        virtual Identifier id() const { return m_identifier; }

        /**
        * @brief Check if the provided identifier matches the stored one.
        *
        * @param id The identifier to check against.
        * @return true if the identifiers match, false otherwise.
        */
        const bool isId(const Identifier& id) const { return m_identifier == id; }

    private:
        Identifier m_identifier; ///< The unique identifier of the DataPackage.
    };

    /**
    * @brief ComplexDataPackage stores multiple types of data required by widgets.
    */
    template<typename Key = uint32_t,typename... Types>
    class ComplexDataPackage : public DataPackage
    {
    public:
        using KeyType = Key;

        /**
        * @brief Constructor.
        *
        * Initializes the ComplexDataPackage with the provided identifier.
        *
        * @param id The identifier for the ComplexDataPackage.
        */
        ComplexDataPackage(Identifier id = 0) : DataPackage(id) {}

        /**
        * @brief Sets a value in the package.
        *
        * @param key The key under which the value will be stored.
        * @param value The value to store.
        */
        template<typename T>
        void set(KeyType key, const T& value)
        {
            static_assert((std::is_same_v<T, Types> || ...), "Type T is not allowed");
            values[key] = value;
        }

        /**
        * @brief Gets a value from the package.
        *
        * @param key The key of the value to retrieve.
        * @return The value associated with the key, or a default-constructed value if the key is not found.
        */
        template<typename T>
        T get(KeyType key) const
        {
            static_assert((std::is_same_v<T, Types> || ...), "Type T is not allowed");
            try
            {
                auto it = values.find(key);
                if( it != values.end() )
                {
                    return std::get<T>(it->second);
                }
            }
            catch( const std::bad_variant_access& e )
            {
                throw std::runtime_error("ComplexDataPackage: " + std::string(e.what())); // Catch and rethrow std::bad_variant_access with a descriptive message
            }
            catch( const std::exception& e )
            {
                throw std::runtime_error("An error occurred in ComplexDataPackage: " + std::string(e.what())); // Catch and rethrow any other exception with a generic error message
            }

            return T();
        }

    protected:
        std::unordered_map<Key, std::variant<Types...>> values; ///< Map to store values of various types.
    };
}
