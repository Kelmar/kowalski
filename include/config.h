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

    /*===============================================================*/

    template <typename T>
    struct Mapper
    {
        typedef typename std::decay<T>::type type;

        void to(type &, Context &) const { static_assert(false); };
    };

    /*===============================================================*/

    namespace details
    {
        template <typename T>
        struct Identity { typedef T type; };

        class Source
        {
        private:
            std::string m_root;

        protected:
            /* constructor */ Source(const std::string_view &root) : m_root(root) { }

            friend config::Context;

#define DECLARE_OPS(T__) \
            virtual void writeValue(const std::string &path, const T__ &value) = 0; \
            virtual void readValue(const std::string &path, T__ &value) = 0;

            DECLARE_OPS(bool)
            DECLARE_OPS(int)
            DECLARE_OPS(size_t)
            DECLARE_OPS(long)
            DECLARE_OPS(float)
            DECLARE_OPS(double)
            DECLARE_OPS(std::string)

#undef DECLARE_OPS

            virtual std::shared_ptr<Context> createContext(bool reading);

        public:
            virtual ~Source() { }

            const std::string &root() const { return m_root; }

            template <typename T>
            void read(const std::string &path, T &value);

            template <typename T>
            void write(const std::string &path, T &value);
        };
    }

    /*===============================================================*/

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
        void map(const std::string &path, T &value)
        {
            mapValue(path, value, details::Identity<T>());
        }

    private:
        template <typename T>
        std::enable_if_t<!std::is_enum_v<T>>
        mapValue(const std::string &path, T &value, details::Identity<T>)
        {
            std::string lastPath = m_currentPath;

            auto d = defer([this, lastPath] () { m_currentPath = lastPath; });

            m_currentPath = pushPath(path);

            Mapper<T> map;
            map.to(value, *this);
        }

        template <typename T>
        std::enable_if_t<std::is_enum_v<T>>
        mapValue(const std::string &path, T &value, details::Identity<T>)
        {
            std::string fullPath = pushPath(path);

            int v = (int)value;

            if (m_reading)
            {
                m_source->readValue(fullPath, v);
                value = (T)v;
            }
            else
            {
                m_source->writeValue(fullPath, v);
            }
        }

        void mapValue(const std::string &path, bool &value,
            details::Identity<bool>)
        {
            std::string fullPath = pushPath(path);

            if (m_reading)
                m_source->readValue(fullPath, value);
            else
                m_source->writeValue(fullPath, value);
        }

        void mapValue(const std::string &path, int &value,
            details::Identity<int>)
        {
            std::string fullPath = pushPath(path);

            if (m_reading)
                m_source->readValue(fullPath, value);
            else
                m_source->writeValue(fullPath, value);
        }

        void mapValue(const std::string &path, unsigned int &value,
                details::Identity<unsigned int>)
        {
            std::string fullPath = pushPath(path);

            int v = value;

            if (m_reading)
            {
                m_source->readValue(fullPath, v);
                value = (unsigned int)v;
            }
            else
                m_source->writeValue(fullPath, v);
        }

        /*std::enable_if_t<!std::is_same_v<int, uint32_t>>
        mapValue(const std::string &path, uint32_t &value,
            details::Identity<uint32_t>)
        {
            std::string fullPath = pushPath(path);

            int v = value;

            if (m_reading)
            {
                m_source->readValue(fullPath, v);
                value = (uint32_t)v;
            }
            else
                m_source->writeValue(fullPath, v);
        }*/

        void mapValue(const std::string &path, std::string &value,
            details::Identity<std::string>)
        {
            std::string fullPath = pushPath(path);

            if (m_reading)
                m_source->readValue(fullPath, value);
            else
                m_source->writeValue(fullPath, value);
        }
    };

    /*===============================================================*/

    inline
    std::shared_ptr<Context> details::Source::createContext(bool reading)
    {
        return std::shared_ptr<Context>(new Context(this, m_root, reading));
    }

    /*===============================================================*/

    template <typename T>
    void details::Source::read(const std::string &path, T &value)
    {
        auto ctx = createContext(true);
        ctx->map(path, value);
    }

    template <typename T>
    void details::Source::write(const std::string &path, T &value)
    {
        auto ctx = createContext(false);
        ctx->map(path, value);
    }

    /*===============================================================*/

    namespace source
    {
#ifdef WIN32

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
            virtual void writeValue(const std::string &, const T__ &) override { } \
            virtual void readValue(const std::string &path, T__ &value) override;

            DECLARE_OPS(bool)
            DECLARE_OPS(int)
            DECLARE_OPS(size_t)
            DECLARE_OPS(long)
            DECLARE_OPS(float)
            DECLARE_OPS(double)
            DECLARE_OPS(std::string)

#undef DECLARE_OPS

        public:
            /* constructor */ WinRegistry(const std::string_view &root);
        };

#endif /* WIN32 */

        /*===============================================================*/

        class wx : public config::details::Source
        {
        private:
            wxConfigBase *m_config;
            wxString m_oldPath;

        protected:
            virtual void writeValue(const std::string &path, const std::string &value)
            {
                m_config->Write(wxString(path), wxString(value));
            }

            virtual void readValue(const std::string &path, std::string &value)
            {
                wxString tmp;

                bool rval = m_config->Read(path, &tmp);

                if (rval)
                    value = tmp;
            }

#define DECLARE_OPS(T__) \
            virtual void writeValue(const std::string &path, const T__ &value) override { m_config->Write(path, value); } \
            virtual void readValue(const std::string &path, T__ &value) override { m_config->Read(path, &value); }

            DECLARE_OPS(bool)
            DECLARE_OPS(int)
            DECLARE_OPS(long)
            DECLARE_OPS(size_t)
            DECLARE_OPS(float)
            DECLARE_OPS(double)
#undef DECLARE_OPS

        public:
            /* constructor */ wx(const std::string_view &root)
                : Source(root)
            {
                m_config = wxConfigBase::Get();
                m_oldPath = m_config->GetPath();
                m_config->SetPath(std::string(root));
            }

            virtual ~wx()
            {
                m_config->SetPath(m_oldPath);
            }
        };
    }

    /*===============================================================*/
}

/*=======================================================================*/

#endif /* CONFIG_6502_H__ */

/*=======================================================================*/
