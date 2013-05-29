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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;

namespace Xceed.Wpf.Toolkit.PropertyGrid
{
  /// <summary>
  /// Used when properties are provided using a list source of items (eg. Properties or PropertiesSource). 
  /// 
  /// An instance of this class can be used as an item to easily customize the 
  /// display of the property directly by modifying the values of this class 
  /// (e.g., DisplayName, value, Category, etc.).
  /// </summary>
  public class CustomPropertyItem : PropertyItemBase
  {

    internal CustomPropertyItem() { }

    #region Category

    public static readonly DependencyProperty CategoryProperty =
        DependencyProperty.Register( "Category", typeof( string ), typeof( CustomPropertyItem ), new UIPropertyMetadata( null ) );

    public string Category
    {
      get { return ( string )GetValue( CategoryProperty ); }
      set { SetValue( CategoryProperty, value ); }
    }

    #endregion //Category

    #region Value

    public static readonly DependencyProperty ValueProperty = DependencyProperty.Register( "Value", typeof( object ), typeof( CustomPropertyItem ), new UIPropertyMetadata( null, OnValueChanged, OnCoerceValueChanged ) );
    public object Value
    {
      get
      {
        return ( object )GetValue( ValueProperty );
      }
      set
      {
        SetValue( ValueProperty, value );
      }
    }

    private static object OnCoerceValueChanged( DependencyObject o, object baseValue )
    {
      CustomPropertyItem prop = o as CustomPropertyItem;
      if( prop != null )
        return prop.OnCoerceValueChanged( baseValue );

      return baseValue;
    }

    protected virtual object OnCoerceValueChanged( object baseValue )
    {
      return baseValue;
    }

    private static void OnValueChanged( DependencyObject o, DependencyPropertyChangedEventArgs e )
    {
      CustomPropertyItem propertyItem = o as CustomPropertyItem;
      if( propertyItem != null )
      {
        propertyItem.OnValueChanged( ( object )e.OldValue, ( object )e.NewValue );
      }
    }

    protected virtual void OnValueChanged( object oldValue, object newValue )
    {
      if( IsInitialized )
      {
        RaiseEvent( new PropertyValueChangedEventArgs( PropertyGrid.PropertyValueChangedEvent, this, oldValue, newValue ) );
      }
    }

    #endregion //Value
  }
}
