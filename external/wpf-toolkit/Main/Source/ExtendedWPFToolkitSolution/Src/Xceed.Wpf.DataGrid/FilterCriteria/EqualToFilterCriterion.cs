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
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Linq;
using System.Linq.Expressions;

namespace Xceed.Wpf.DataGrid.FilterCriteria
{
  [CriterionDescriptor( "=@", FilterOperatorPrecedence.RelationalOperator, FilterTokenPriority.RelationalOperator )]
  [DebuggerDisplay( "EqualTo( {Value} )" )]
  public class EqualToFilterCriterion : RelationalFilterCriterion
  {
    public EqualToFilterCriterion()
      : base()
    {
    }

    public EqualToFilterCriterion( object value )
      : base( value )
    {
    }

    public override string ToExpression( CultureInfo culture )
    {
      string strValue = this.GetValueForExpression( culture );
      return ( strValue == null ) ? "" : "=" + strValue;
    }

    public override Expression ToLinqExpression( IQueryable queryable, ParameterExpression parameterExpression, string propertyName )
    {
      if( queryable == null )
        throw new ArgumentNullException( "queryable" );

      if( parameterExpression == null )
        throw new ArgumentNullException( "parameterExpression" );

      if( String.IsNullOrEmpty( propertyName ) )
      {
        if( propertyName == null )
          throw new ArgumentNullException( "propertyName" );

        throw new ArgumentException( "PropertyName must not be empty.", "propertyName" );
      }

      return queryable.CreateEqualExpression( parameterExpression, propertyName, this.Value );
    }

    public override bool IsMatch( object value )
    {
      return ( this.Compare( value ) == CompareResult.EqualsTo );
    }

#if DEBUG
    public override string ToString()
    {
      StringBuilder stringBuilder = new StringBuilder( "EqualTo( " );
      stringBuilder.Append( ( this.Value == null ) ? "#NULL#" : this.Value.ToString() );
      stringBuilder.Append( " )" );

      return stringBuilder.ToString();
    }
#endif
  }
}
