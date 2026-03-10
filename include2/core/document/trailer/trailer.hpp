#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <string>
#include <utility>

namespace ripper::core
{
    /**
     * @brief Represents a pdf trailer dictionary.
     *
     * The trailer contains important document-level information like the root catalog,
     * document info, encryption settings, and references to previous revisions.
     */
    class trailer
    {
    public:
        trailer() = default;

        /**
         * @brief Sets the /size entry (total number of entries in the xref table).
         */
        void set_size(std::uint32_t size);

        /**
         * @brief Gets the /size entry.
         */
        [[nodiscard]] std::optional<std::uint32_t> size() const;

        /**
         * @brief Sets the /prev entry (offset to previous xref table).
         */
        void set_prev(std::size_t prev);

        /**
         * @brief Gets the /prev entry.
         */
        [[nodiscard]] std::optional<std::size_t> prev() const;

        /**
         * @brief Sets the /Root entry (catalog object reference).
         */
        void set_root(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Root object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> root_object_number() const;

        /**
         * @brief Gets the /Root generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> root_generation() const;

        /**
         * @brief Sets the /Info entry (document info object reference).
         */
        void set_info(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Info object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> info_object_number() const;

        /**
         * @brief Gets the /Info generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> info_generation() const;

        /**
         * @brief Sets the /Encrypt entry (encryption dictionary object reference).
         */
        void set_encrypt(std::uint32_t objectNumber, std::uint16_t generation);

        /**
         * @brief Gets the /Encrypt object number.
         */
        [[nodiscard]] std::optional<std::uint32_t> encrypt_object_number() const;

        /**
         * @brief Gets the /Encrypt generation number.
         */
        [[nodiscard]] std::optional<std::uint16_t> encrypt_generation() const;

        /**
         * @brief Sets the /id array (file identifier).
         * @param ids Pair of byte strings. First is the original document id (required),
         *            second is the current revision id (optional).
         */
        void set_id(const std::pair<std::string, std::optional<std::string>> &ids);

        /**
         * @brief Gets the /id array.
         * @return Pair containing original id and optional current revision id.
         */
        [[nodiscard]] const std::optional<std::pair<std::string, std::optional<std::string>>> &id() const;

        /**
         * @brief Merges another trailer into this one.
         * Newer values (from other) override existing values.
         */
        void merge(const trailer &other);

    private:
        std::optional<std::uint32_t> _size;
        std::optional<std::size_t> _prev;
        std::optional<std::uint32_t> _rootObjectNumber;
        std::optional<std::uint16_t> _rootGeneration;
        std::optional<std::uint32_t> _infoObjectNumber;
        std::optional<std::uint16_t> _infoGeneration;
        std::optional<std::uint32_t> _encryptObjectNumber;
        std::optional<std::uint16_t> _encryptGeneration;
        std::optional<std::pair<std::string, std::optional<std::string>>> _id;
    };
}
