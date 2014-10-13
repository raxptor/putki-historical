#ifndef __INDENTEDWRITER_H__
#define __INDENTEDWRITER_H__

#include <iostream>

namespace putki
{
	class indentedwriter
	{
public:
		indentedwriter(std::ostream &ostr) : m_stream(ostr), m_indent(0)
		{

		}

		void indent(int mod)
		{
			m_indent += mod;
		}

		std::ostream& line(int indentmod=0)
		{
			m_stream << std::endl;
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
	};
}

#endif
