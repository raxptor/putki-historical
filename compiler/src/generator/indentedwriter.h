#ifndef __INDENTEDWRITER_H__
#define __INDENTEDWRITER_H__

#include <iostream>

namespace putki
{
	class indentedwriter
	{
public:
		indentedwriter(std::ostream &ostr) : m_stream(ostr), m_first(false), m_indent(0)
		{

		}

		void indent(int mod)
		{
			m_indent += mod;
		}

		std::ostream& line(int indentmod=0)
		{
			if (!m_first)
				m_stream << std::endl;

			m_first = false;
			for (int i=0; i<(m_indent+indentmod); i++)
				m_stream << "\t";

			return m_stream;
		}

		std::ostream& cont()
		{
			return m_stream;
		}

private:

		std::ostream& m_stream;
		int m_indent;
		int m_first;
	};
}

#endif
