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
using System.Text;
using System.Windows.Markup;
using System.Collections.ObjectModel;
using System.Windows.Data;

namespace Xceed.Wpf.DataGrid
{
  [ContentProperty( "SelectorItems" )]
  public class LevelGroupConfigurationSelector : GroupConfigurationSelector
  {
    private LevelGroupConfigurationSelectorItemCollection m_groupConfigurationSelectorItems = new LevelGroupConfigurationSelectorItemCollection();

    public ObservableCollection<LevelGroupConfigurationSelectorItem> SelectorItems
    {
      get
      {
        return m_groupConfigurationSelectorItems;
      }
    }

    public override GroupConfiguration SelectGroupConfiguration(int groupLevel, CollectionViewGroup collectionViewGroup, System.ComponentModel.GroupDescription groupDescription)
    {
      if (m_groupConfigurationSelectorItems.Count == 0)
        return base.SelectGroupConfiguration( groupLevel, collectionViewGroup, groupDescription );

      LevelGroupConfigurationSelectorItem levelGroupConfig = m_groupConfigurationSelectorItems.GetGroupConfigurationSelectorItem( groupLevel );
      if( levelGroupConfig != null )
      {
        return levelGroupConfig.GroupConfiguration;
      }

      return base.SelectGroupConfiguration( groupLevel, collectionViewGroup, groupDescription );
    }
  }
}
