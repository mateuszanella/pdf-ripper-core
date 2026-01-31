#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <string>

namespace Ripper::Core
{
    /**
     * @brief Represents a PDF trailer dictionary.
     *
     * The trailer contains important document-level information like the root catalog,
     * document info, encryption settings, and references to previous revisions.
     */
    class Trailer
    {
    public:
        Trailer() = default;

        /**
         * @brief Sets the /Size entry (total number of entries in the xref table).
         */
        void SetSize(std::uint32_t size);

        /**
         * @brief Gets the /Size entry.
         */
        [[nodiscard]] std::optional<std::uint32_t> Size() const;

        /**
         * @brief Sets the /Prev entry (offset to previous xref table).
         */
        void SetPrev(std::size_t prev);

        /**
         * @brief Gets the /Prev entry.
         */
        [[nodiscard]] std::optional<std::size_t> Prev() const;

        /**
         * @brief Sets the /Root entry (catalog object reference).
         */
        void SetRoot(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Root object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> RootObjectNumber() const;

        /**
         * @brief Gets the /Root generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> RootGeneration() const;

        /**
         * @brief Sets the /Info entry (document info object reference).
         */
        void SetInfo(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Info object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> InfoObjectNumber() const;

        /**
         * @brief Gets the /Info generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> InfoGeneration() const;

        /**
         * @brief Sets the /Encrypt entry (encryption dictionary object reference).
         */
        void SetEncrypt(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Encrypt object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> EncryptObjectNumber() const;

        /**
         * @brief Gets the /Encrypt generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> EncryptGeneration() const;

        /**
         * @brief Sets the /ID array (file identifier).
         */
        void SetID(const std::string &id);

        /**
         * @brief Gets the /ID entry.
         */
        [[nodiscard]] const std::optional<std::string> &ID() const;

        /**
         * @brief Merges another trailer into this one.
         * Newer values (from other) override existing values.
         */
        void Merge(const Trailer &other);

    private:
        std::optional<std::uint32_t> _size;
        std::optional<std::size_t> _prev;
        std::optional<std::uint32_t> _rootObjectNumber;
        std::optional<std::uint16_t> _rootGeneration;
        std::optional<std::uint32_t> _infoObjectNumber;
        std::optional<std::uint16_t> _infoGeneration;
        std::optional<std::uint32_t> _encryptObjectNumber;
        std::optional<std::uint16_t> _encryptGeneration;
        std::optional<std::string> _id;
    };
}
