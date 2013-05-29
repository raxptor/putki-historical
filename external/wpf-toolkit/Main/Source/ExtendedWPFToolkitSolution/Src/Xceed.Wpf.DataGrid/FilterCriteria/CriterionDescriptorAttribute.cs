﻿/*************************************************************************************

   Extended WPF Toolkit

   Copyright (C) 2007-2013 Xceed Software Inc.

   This program is provided to you under the terms of the Microsoft Public
   License (Ms-PL) as published at http://wpftoolkit.codeplex.com/license 

   For more features, controls, and fast professional support,
   pick up the Plus Edition at http://xceed.com/wpf_toolkit

   Stay informed: follow @datagrid on Twitter or Like http://facebook.com/datagrids

  ***********************************************************************************/

using System;

namespace Xceed.Wpf.DataGrid.FilterCriteria
{
  [ AttributeUsage( AttributeTargets.Class, Inherited = true, AllowMultiple = false ) ]
  internal sealed class CriterionDescriptorAttribute : Attribute
  {
    public CriterionDescriptorAttribute( string pattern, FilterOperatorPrecedence operatorPrecedence, FilterTokenPriority parserPriority )
    {
      m_pattern = pattern;
      m_operatorPrecedence = operatorPrecedence;
      m_parserPriority = parserPriority;
    }

    public string Pattern
    {
      get
      {
        return m_pattern;
      }
    }

    public FilterOperatorPrecedence OperatorPrecedence
    {
      get
      {
        return m_operatorPrecedence;
      }
    }

    public FilterTokenPriority ParserPriority
    {
      get
      {
        return m_parserPriority;
      }
    }

    private readonly string m_pattern;
    private readonly FilterOperatorPrecedence m_operatorPrecedence;
    private readonly FilterTokenPriority m_parserPriority;
  }
}
