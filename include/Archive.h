/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
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
 * @brief Check function
 * 
 * @remarks This function will be called while reading/writing bytes to
 * accumulate a running check of the file's bytes (if desired).
 */
typedef std::function<void(const std::span<uint8_t> &bytes)> CheckFn;

/**
 * @brief Class for working with binary formatted files.
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

private:
    typedef std::function<void(_Inout_ std::span<uint8_t> &)> SwapFn;

    std::string m_filename;
    std::FILE *m_file;
    Mode m_mode;
    Endianess m_endianess;
    CheckFn m_checkFn;
    SwapFn m_swapFn;

    // Dummy function that does not perform a check
    static inline void noCheck(const std::span<uint8_t> &) { }

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
    /* constructor */ Archive(
        const std::string &filename, 
        Mode mode, 
        Endianess endianess = Endianess::None,
        CheckFn checkFn = noCheck);

    virtual ~Archive();

    std::string filename() const { return m_filename; }

    Mode mode() const { return m_mode; }

    Endianess endianess() const { return m_endianess; }

    bool eof() const { return std::feof(m_file); }

    // Read raw data into buffer
    size_t read(std::span<uint8_t> buffer);

    // Write raw buffer data
    void write(const std::span<uint8_t> &buffer);

    template <typename T>
    void operation(T &val)
    {
        if (m_mode == Mode::Read)
        {
            uint8_t *b = reinterpret_cast<uint8_t *>(&val);
            std::span<uint8_t> s(b, sizeof(T));

            read(s);
            m_swapFn(s);
        }
        else
        {
            T v1 = val; // Make copy that we can modify.

            uint8_t *b = reinterpret_cast<uint8_t *>(&v1);
            std::span<uint8_t> s(b, sizeof(T));

            m_swapFn(s);
            write(s);
        }
    }
};

/*************************************************************************/

template <typename T>
Archive &operator &(Archive &file, T &val)
{
    file.operation(val);
    return file;
}

/*************************************************************************/

#endif /* ARCHIVE_H__ */

/*************************************************************************/
