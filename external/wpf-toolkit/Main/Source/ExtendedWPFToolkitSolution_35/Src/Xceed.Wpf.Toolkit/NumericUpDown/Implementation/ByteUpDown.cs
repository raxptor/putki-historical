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
using System.Windows;

namespace Xceed.Wpf.Toolkit
{
  public class ByteUpDown : CommonNumericUpDown<byte>
  {
    #region Constructors

    static ByteUpDown()
    {
      UpdateMetadata( typeof( ByteUpDown ), ( byte )1, byte.MinValue, byte.MaxValue );
    }

    public ByteUpDown()
      : base( Byte.Parse, Decimal.ToByte, ( v1, v2 ) => v1 < v2, ( v1, v2 ) => v1 > v2 )
    {
    }

    #endregion //Constructors

    #region Base Class Overrides

    protected override byte IncrementValue( byte value, byte increment )
    {
      return ( byte )( value + increment );
    }

    protected override byte DecrementValue( byte value, byte increment )
    {
      return ( byte )( value - increment );
    }

    #endregion //Base Class Overrides
  }
}
