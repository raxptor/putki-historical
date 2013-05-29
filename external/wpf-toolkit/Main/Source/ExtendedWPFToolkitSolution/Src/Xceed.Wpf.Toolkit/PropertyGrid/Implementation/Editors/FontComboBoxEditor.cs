﻿/*************************************************************************************

   Extended WPF Toolkit

   Copyright (C) 2007-2013 Xceed Software Inc.

   This program is provided to you under the terms of the Microsoft Public
   License (Ms-PL) as published at http://wpftoolkit.codeplex.com/license 

   For more features, controls, and fast professional support,
   pick up the Plus Edition at http://xceed.com/wpf_toolkit

   Stay informed: follow @datagrid on Twitter or Like http://facebook.com/datagrids

  ***********************************************************************************/

using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Media;
using System.Collections;
using Xceed.Wpf.Toolkit.Core.Utilities;

namespace Xceed.Wpf.Toolkit.PropertyGrid.Editors
{
  public class FontComboBoxEditor : ComboBoxEditor
  {
    protected override IEnumerable CreateItemsSource( PropertyItem propertyItem )
    {
      if( propertyItem.PropertyType == typeof( FontFamily ) )
        return FontUtilities.Families;
      else if( propertyItem.PropertyType == typeof( FontWeight ) )
        return FontUtilities.Weights;
      else if( propertyItem.PropertyType == typeof( FontStyle ) )
        return FontUtilities.Styles;
      else if( propertyItem.PropertyType == typeof( FontStretch ) )
        return FontUtilities.Stretches;

      return null;
    }
  }
}
