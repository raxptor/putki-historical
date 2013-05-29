﻿/*************************************************************************************

   Extended WPF Toolkit

   Copyright (C) 2007-2013 Xceed Software Inc.

   This program is provided to you under the terms of the Microsoft Public
   License (Ms-PL) as published at http://wpftoolkit.codeplex.com/license 

   For more features, controls, and fast professional support,
   pick up the Plus Edition at http://xceed.com/wpf_toolkit

   Stay informed: follow @datagrid on Twitter or Like http://facebook.com/datagrids

  ***********************************************************************************/

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using Xceed.Wpf.Toolkit.Core.Utilities;

namespace Xceed.Wpf.Toolkit
{
  [TemplatePart( Name = PART_VisualBrush, Type = typeof( VisualBrush ) )]
  public class Magnifier : Control
  {
    private const double DEFAULT_SIZE = 100d;
    private const string PART_VisualBrush = "PART_VisualBrush";

    #region Private Members

    private VisualBrush _visualBrush = new VisualBrush();

    #endregion //Private Members

    #region Properties

    #region FrameType

    public static readonly DependencyProperty FrameTypeProperty = DependencyProperty.Register( "FrameType", typeof( FrameType ), typeof( Magnifier ), new UIPropertyMetadata( FrameType.Circle, OnFrameTypeChanged ) );
    public FrameType FrameType
    {
      get
      {
        return ( FrameType )GetValue( FrameTypeProperty );
      }
      set
      {
        SetValue( FrameTypeProperty, value );
      }
    }

    private static void OnFrameTypeChanged( DependencyObject d, DependencyPropertyChangedEventArgs e )
    {
      Magnifier m = ( Magnifier )d;
      m.OnFrameTypeChanged( ( FrameType )e.OldValue, ( FrameType )e.NewValue );
    }

    protected virtual void OnFrameTypeChanged( FrameType oldValue, FrameType newValue )
    {
      this.UpdateSizeFromRadius();
    }

    #endregion //FrameType

    #region Radius

    public static readonly DependencyProperty RadiusProperty = DependencyProperty.Register( "Radius", typeof( double ), typeof( Magnifier ), new FrameworkPropertyMetadata( ( Magnifier.DEFAULT_SIZE / 2 ), new PropertyChangedCallback( OnRadiusPropertyChanged ) ) );
    public double Radius
    {
      get
      {
        return ( double )GetValue( RadiusProperty );
      }
      set
      {
        SetValue( RadiusProperty, value );
      }
    }

    private static void OnRadiusPropertyChanged( DependencyObject d, DependencyPropertyChangedEventArgs e )
    {
      Magnifier m = ( Magnifier )d;
      m.OnRadiusChanged( e );
    }

    protected virtual void OnRadiusChanged( DependencyPropertyChangedEventArgs e )
    {
      this.UpdateSizeFromRadius();
    }

    #endregion

    #region Target

    public static readonly DependencyProperty TargetProperty = DependencyProperty.Register( "Target", typeof( UIElement ), typeof( Magnifier ) );
    public UIElement Target
    {
      get
      {
        return ( UIElement )GetValue( TargetProperty );
      }
      set
      {
        SetValue( TargetProperty, value );
      }
    }

    #endregion //Target

    #region ViewBox

    internal Rect ViewBox
    {
      get
      {
        return _visualBrush.Viewbox;
      }
      set
      {
        _visualBrush.Viewbox = value;
      }
    }

    #endregion

    #region ZoomFactor

    public static readonly DependencyProperty ZoomFactorProperty = DependencyProperty.Register( "ZoomFactor", typeof( double ), typeof( Magnifier ), new FrameworkPropertyMetadata( 0.5, OnZoomFactorPropertyChanged), OnValidationCallback );
    public double ZoomFactor
    {
      get
      {
        return ( double )GetValue( ZoomFactorProperty );
      }
      set
      {
        SetValue( ZoomFactorProperty, value );
      }
    }

    private static bool OnValidationCallback( object baseValue )
    {
      double zoomFactor = ( double )baseValue;
      return ( zoomFactor >= 0 );
    }

    private static void OnZoomFactorPropertyChanged( DependencyObject d, DependencyPropertyChangedEventArgs e )
    {
      Magnifier m = ( Magnifier )d;
      m.OnZoomFactorChanged( e );
    }

    protected virtual void OnZoomFactorChanged( DependencyPropertyChangedEventArgs e )
    {
      UpdateViewBox();
    }

    #endregion //ZoomFactor

    #endregion //Properties

    #region Constructors

    /// <summary>
    /// Initializes static members of the <see cref="Magnifier"/> class.
    /// </summary>
    static Magnifier()
    {
      DefaultStyleKeyProperty.OverrideMetadata( typeof( Magnifier ), new FrameworkPropertyMetadata( typeof( Magnifier ) ) );
      HeightProperty.OverrideMetadata( typeof( Magnifier ), new FrameworkPropertyMetadata( Magnifier.DEFAULT_SIZE ) );
      WidthProperty.OverrideMetadata( typeof( Magnifier ), new FrameworkPropertyMetadata( Magnifier.DEFAULT_SIZE ) );
    }

    public Magnifier()
    {
      this.SizeChanged += new SizeChangedEventHandler( OnSizeChangedEvent );
    }

    private void OnSizeChangedEvent( object sender, SizeChangedEventArgs e )
    {
      UpdateViewBox();
    }

    private void UpdateSizeFromRadius()
    {
      if( this.FrameType == Toolkit.FrameType.Circle )
      {
        double newSize = Radius * 2;
        if(!DoubleHelper.AreVirtuallyEqual( Width, newSize ))
        {
          Width = newSize;
        }

        if(!DoubleHelper.AreVirtuallyEqual( Height, newSize ))
        {
          Height = newSize;
        }
      }
    }

    #endregion

    #region Base Class Overrides

    public override void OnApplyTemplate()
    {
      base.OnApplyTemplate();

      VisualBrush newBrush = GetTemplateChild( PART_VisualBrush ) as VisualBrush;

      // Just create a brush as placeholder even if there is no such brush.
      // This avoids having to "if" each access to the _visualBrush member.
      // Do not keep the current _visualBrush whatsoever to avoid memory leaks.
      if( newBrush == null )
      {
        newBrush = new VisualBrush();
      }

      newBrush.Viewbox = _visualBrush.Viewbox;
      _visualBrush = newBrush;
    }

    #endregion // Base Class Overrides

    #region Methods

    private void UpdateViewBox()
    {
      if( !IsInitialized )
        return;

      ViewBox = new Rect( 
        ViewBox.Location, 
        new Size( ActualWidth * ZoomFactor , ActualHeight * ZoomFactor ) );
    }

    #endregion //Methods
  }
}
