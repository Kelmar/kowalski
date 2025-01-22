/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#ifndef CONFIG_6502_H__
#define CONFIG_6502_H__

/*=======================================================================*/

namespace config
{
    class Context;

    static const char PATH_SEPERATOR = '/';

    /****************************************************************/

    template <typename T>
    struct Mapper
    {
        typedef typename std::decay<T>::type type;

        bool to(type &, Context &) const { return false; };
    };

    /****************************************************************/

    namespace details
    {
        template <typename T> struct Identity { typedef T type; };

        class Source
        {
        private:
            std::string m_root;

        protected:
            /* constructor */ Source(const std::string_view &root) : m_root(root) { }

            friend config::Context;

#define DECLARE_OPS(T__) \
            virtual bool writeValue(const std::string &path, const T__ &value) = 0; \
            virtual bool readValue(const std::string &path, T__ &value) = 0;

            //DECLARE_OPS(bool)
            DECLARE_OPS(int)
            //DECLARE_OPS(unsigned int)
            //DECLARE_OPS(float)
            //DECLARE_OPS(double)
            DECLARE_OPS(std::string)

#undef DECLARE_OPS

            virtual std::shared_ptr<Context> createContext(bool reading);

        public:
            virtual ~Source() { }

            const std::string &root() const { return m_root; }

            template <typename T>
            bool read(const std::string &path, T &value);

            template <typename T>
            bool write(const std::string &path, T &value);
        };
    }

    /****************************************************************/

    class Context
    {
    private:
        Context(const Context &) = delete;
        Context(Context &&) = delete;

        const Context &operator =(const Context &) = delete;
        const Context &operator =(Context &&) = delete;

    private:
        details::Source *m_source;
        bool m_reading;

        std::string m_currentPath;

        std::string pushPath(const std::string &path)
        {
            return m_currentPath + PATH_SEPERATOR + path;
        }

    protected:
        friend class details::Source;

        Context(details::Source *src, const std::string_view &path, bool reading)
            : m_source(src)
            , m_reading(reading)
            , m_currentPath(path)
        { }

    public:
        virtual ~Context() { }

        template <typename T>
        bool map(const std::string &path, T &value)
        {
            return mapValue(path, value, details::Identity<T>());
        }

    private:
        template <typename T>
        bool mapValue(const std::string &path, T &value, details::Identity<T>)
        {
            std::string lastPath = m_currentPath;

            auto d = defer([this, lastPath] () { m_currentPath = lastPath; });

            m_currentPath = pushPath(path);

            Mapper<T> map;
            return map.to(value, *this);
        }

        bool mapValue(const std::string &path, int &value,
            details::Identity<int>)
        {
            std::string fullPath = pushPath(path);

            return m_reading ?
                m_source->readValue(fullPath, value) :
                m_source->writeValue(fullPath, value);
        }

        bool mapValue(const std::string &path, std::string &value,
            details::Identity<std::string>)
        {
            std::string fullPath = pushPath(path);

            return m_reading ?
                m_source->readValue(fullPath, value) :
                m_source->writeValue(fullPath, value);
        }
    };

    /****************************************************************/

    inline
    std::shared_ptr<Context> details::Source::createContext(bool reading)
    {
        return std::shared_ptr<Context>(new Context(this, m_root, reading));
    }

    /****************************************************************/

    template <typename T>
    bool details::Source::read(const std::string &path, T &value)
    {
        auto ctx = createContext(true);
        return ctx->map(path, value);
    }

    template <typename T>
    bool details::Source::write(const std::string &path, T &value)
    {
        auto ctx = createContext(false);
        return ctx->map(path, value);
    }

    /****************************************************************/

#ifdef WIN32

    namespace source
    {
        /**
         * @brief Config source for reading legacy configuration.
         */
        class WinRegistry : public config::details::Source
        {
        private:
            HKEY m_key;

            void createKey(bool reading);
            void releaseKey();

            bool readReg(const std::string &path, DWORD type, _Out_ std::vector<BYTE> &buffer);

            virtual std::shared_ptr<Context> createContext(bool reading) override;

        protected:
            // We only support reading from this source.

#define DECLARE_OPS(T__) \
            virtual bool writeValue(const std::string &, const T__ &) override { return false; } \
            virtual bool readValue(const std::string &path, T__ &value) override;

            //DECLARE_OPS(bool)
            DECLARE_OPS(int)
            //DECLARE_OPS(unsigned int)
            //DECLARE_OPS(float)
            //DECLARE_OPS(double)
            DECLARE_OPS(std::string)

#undef DECLARE_OPS

        public:
            /* constructor */ WinRegistry(const std::string_view &root);
        };
    }

#endif /* WIN32 */

    /****************************************************************/
}

/*=======================================================================*/

#endif /* CONFIG_6502_H__ */

/*=======================================================================*/
