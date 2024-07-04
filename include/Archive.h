/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the �Software�),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#ifndef ARCHIVE_H__
#define ARCHIVE_H__

/*************************************************************************/
/**
 * @brief Class for working with files.
 */
class Archive
{
public:
    /**
     * @brief The mode that we've opened the file in reading or writing.
     */
    enum class Mode
    {
        Read,
        Write
    };

private:
    std::string m_filename;
    std::FILE *m_file;
    size_t m_size;
    Mode m_mode;

protected:
    /* constructor */ Archive(const std::string &filename, Mode mode);

public:
    virtual ~Archive();

    /**
     * @brief Returns the name of the file.
     */
    std::string filename() const { return m_filename; }

    /**
     * Returns if the archive is in read or write mode.
     */
    Mode mode() const { return m_mode; }

    /**
     * @brief Returns flag indicating if the end of the file has been reached.
     * @remarks Only valid if file is open for reading.
     */
    bool eof() const { return std::feof(m_file); }

    /**
     * @brief Returns the size of the file in bytes.
     * @remarks When open for reading, this is the size on disk.
     * When open for writing, this is the number of bytes written so far.
     */
    size_t size() const { return m_size; }

    // Read raw data into buffer
    size_t read(std::span<uint8_t> buffer);

    // Write raw buffer data
    void write(const std::span<uint8_t> &buffer);
};

/*************************************************************************/
/**
 * @brief Class for working with text based file formats.
 */
class TextArchive : public Archive
{
public:
    enum class LineEnding
    {
        DOS  = 0, // CR+LF  \r\n
        UNIX = 1, // LF     \n
        MAC  = 2  // CR     \r      (Classic Mac, OS X uses Unix style)
    };

#ifdef WIN32
    static constexpr LineEnding LE_DEFAULT = LineEnding::DOS;
#else
    static constexpr LineEnding LE_DEFAULT = LineEnding::UNIX;
#endif

private:
    LineEnding m_lineEnding;

public:
    TextArchive(const std::string &filename,
        Archive::Mode mode,
        LineEnding lineEnding = LE_DEFAULT);

    std::string readLine();

    void writeLine(const std::string &line);
};

/*************************************************************************/
/**
 * @brief Class for working with binary based file formats.
 */
class BinaryArchive : public Archive
{
public:
    /**
     * @brief How word, double word, and quad word values should be interpreted.
     */
    enum class Endianess
    {
        /**
         * @brief Not interpreted
         *
         * @remarks Uses the host platform's native format.
         */
        None,

        /**
         * @brief Little endian format
         *
         * @remarks LSB first
         */
        Little,

        /**
         * @brief Big endian format
         *
         * @remarks MSB first
         */
        Big
    };

protected:
    typedef std::function<void(_Inout_ std::span<uint8_t> &)> SwapFn;

    Endianess m_endianess;
    SwapFn m_swapFn;

    // Do nothing
    static inline void noSwap(_Inout_ std::span<uint8_t> &) { }

    // Swap bytes to different endian type.
    static inline void byteSwap(_Inout_ std::span<uint8_t> &bytes)
    {
        for (size_t i = 0, s = bytes.size(); i < s / 2; ++i)
        {
            bytes[i] ^= bytes[s - i];
            bytes[s - i] ^= bytes[i];
            bytes[i] ^= bytes[s - i];
        }
    }

public:
    BinaryArchive(const std::string &filename,
        Mode mode,
        Endianess endianess = Endianess::None);

    Endianess endianess() const { return m_endianess; }

    // Operation for types that need endian conversion.
    template <typename T>
    void operation(T &val, std::true_type)
    {
        if (mode() == Mode::Read)
        {
            uint8_t *b = reinterpret_cast<uint8_t *>(&val);
            std::span<uint8_t> s(b, sizeof(T));

            read(s);
            m_swapFn(s);
        }
        else
        {
            T tmp = val; // Make copy that we can modify.

            uint8_t *b = reinterpret_cast<uint8_t *>(&tmp);
            std::span<uint8_t> s(b, sizeof(T));

            m_swapFn(s);
            write(s);
        }
    }

    // Operation for raw types
    template <typename T>
    void operation(T &val, std::false_type)
    {
        uint8_t *b = reinterpret_cast<uint8_t *>(&val);
        std::span<uint8_t> s(b, sizeof(T));

        if (mode() == Mode::Read)
            read(s);
        else
            write(s);
    }
};

/*************************************************************************/

template <typename T>
BinaryArchive &operator &(BinaryArchive &file, T &val)
{
    file.operation(val, std::is_integral<T>());
    return file;
}

/*************************************************************************/

#endif /* ARCHIVE_H__ */

/*************************************************************************/
