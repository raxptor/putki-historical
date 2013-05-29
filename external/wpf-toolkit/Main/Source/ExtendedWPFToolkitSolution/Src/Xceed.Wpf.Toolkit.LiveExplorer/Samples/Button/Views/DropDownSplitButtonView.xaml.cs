﻿/***************************************************************************************

   Extended WPF Toolkit

   Copyright (C) 2007-2013 Xceed Software Inc.

   This program is provided to you under the terms of the Microsoft Public
   License (Ms-PL) as published at http://wpftoolkit.codeplex.com/license 

   For more features, controls, and fast professional support,
   pick up the Plus Edition at http://xceed.com/wpf_toolkit

   Stay informed: follow @datagrid on Twitter or Like http://facebook.com/datagrids

  *************************************************************************************/

namespace Xceed.Wpf.Toolkit.LiveExplorer.Samples.Button.Views
{
  /// <summary>
  /// Interaction logic for DropDownSplitButtonView.xaml
  /// </summary>
  public partial class DropDownSplitButtonView : DemoView
  {
    public DropDownSplitButtonView()
    {
      InitializeComponent();
    }

    private void DropDownButton_Click( object sender, System.Windows.RoutedEventArgs e )
    {

    }

    private void Button_Click( object sender, System.Windows.RoutedEventArgs e )
    {
      _dropDownButton.IsOpen = false;
      _splitButton.IsOpen = false;
    }

    private void SplitButton_Click( object sender, System.Windows.RoutedEventArgs e )
    {
      Xceed.Wpf.Toolkit.MessageBox.Show( "Thanks for clicking me!", "SplitButton Click" );
    }
  }
}
