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
using System.Windows;

namespace Xceed.Wpf.DataGrid
{
  internal class CanBeCurrentWhenReadOnlyChangedEventManager: WeakEventManager
  {
    private CanBeCurrentWhenReadOnlyChangedEventManager()
    {
    }

    public static void AddListener( ColumnBase source, IWeakEventListener listener )
    {
      CurrentManager.ProtectedAddListener( source, listener );
    }

    public static void RemoveListener( ColumnBase source, IWeakEventListener listener )
    {
      CurrentManager.ProtectedRemoveListener( source, listener );
    }

    protected override void StartListening( object source )
    {
      ColumnBase column = ( ColumnBase )source;
      column.CanBeCurrentWhenReadOnlyChanged += new EventHandler( this.OnCanBeCurrentWhenReadOnlyChanged );
    }

    protected override void StopListening( object source )
    {
      ColumnBase column = ( ColumnBase )source;
      column.CanBeCurrentWhenReadOnlyChanged -= new EventHandler( this.OnCanBeCurrentWhenReadOnlyChanged );
    }

    private static CanBeCurrentWhenReadOnlyChangedEventManager CurrentManager
    {
      get
      {
        Type managerType = typeof( CanBeCurrentWhenReadOnlyChangedEventManager );
        CanBeCurrentWhenReadOnlyChangedEventManager currentManager = ( CanBeCurrentWhenReadOnlyChangedEventManager )WeakEventManager.GetCurrentManager( managerType );

        if( currentManager == null )
        {
          currentManager = new CanBeCurrentWhenReadOnlyChangedEventManager();
          WeakEventManager.SetCurrentManager( managerType, currentManager );
        }

        return currentManager;
      }
    }

    private void OnCanBeCurrentWhenReadOnlyChanged( object sender, EventArgs args )
    {
      this.DeliverEvent( sender, args );
    }
  }
}
