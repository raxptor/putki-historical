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
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;
using System.Windows.Data;
using Xceed.Wpf.DataGrid.Views;
using System.Windows.Input;
using System.Windows.Controls;
using System.Collections.Specialized;
using System.Windows.Threading;
using System.Windows.Media.Animation;
using System.Collections.Generic;

namespace Xceed.Wpf.DataGrid
{
  public class DataRow : Row, IEditableObject
  {
    #region CONSTRUCTORS

    public DataRow()
    {
      this.CommandBindings.Add( new CommandBinding( DataGridCommands.ExpandDetails,
                                                    new ExecutedRoutedEventHandler( OnExpandDetailsExecuted ),
                                                    new CanExecuteRoutedEventHandler( OnExpandDetailsCanExecute ) ) );

      this.CommandBindings.Add( new CommandBinding( DataGridCommands.CollapseDetails,
                                                    new ExecutedRoutedEventHandler( OnCollapseDetailsExecuted ),
                                                    new CanExecuteRoutedEventHandler( OnCollapseDetailsCanExecute ) ) );

      this.CommandBindings.Add( new CommandBinding( DataGridCommands.ToggleDetailExpansion,
                                                    new ExecutedRoutedEventHandler( OnToggleDetailsExecuted ),
                                                    new CanExecuteRoutedEventHandler( OnToggleDetailsCanExecute ) ) );
    }

    #endregion CONSTRUCTORS

    private void OnExpandDetailsExecuted( object sender, ExecutedRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        dataGridContext.ExpandDetails( this.DataContext );
      }
    }

    private void OnExpandDetailsCanExecute( object sender, CanExecuteRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        if( ( dataGridContext.AllowDetailToggle == true ) && ( dataGridContext.HasDetails == true ) && ( DataGridControl.GetHasExpandedDetails( this ) == false ) )
        {
          e.CanExecute = true;
          return;
        }
      }
      e.CanExecute = false;
    }

    private void OnCollapseDetailsExecuted( object sender, ExecutedRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        dataGridContext.CollapseDetails( this.DataContext );
      }
    }

    private void OnCollapseDetailsCanExecute( object sender, CanExecuteRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        if( ( dataGridContext.AllowDetailToggle == true ) && ( dataGridContext.HasDetails == true ) && ( DataGridControl.GetHasExpandedDetails( this ) == true ) )
        {
          e.CanExecute = true;
          return;
        }
      }
      e.CanExecute = false;
    }

    private void OnToggleDetailsExecuted( object sender, ExecutedRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        dataGridContext.ToggleDetailExpansion( this.DataContext );
      }
    }

    private void OnToggleDetailsCanExecute( object sender, CanExecuteRoutedEventArgs e )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );
      if( ( dataGridContext != null ) && ( this.DataContext != null ) )
      {
        if( ( dataGridContext.AllowDetailToggle == true ) && ( dataGridContext.HasDetails == true ) )
        {
          e.CanExecute = true;
          return;
        }
      }
      e.CanExecute = false;
    }

    #region ItemIndex property

    internal int ItemIndex
    {
      get
      {
        return DataGridVirtualizingPanel.GetItemIndex( this );
      }
    }

    #endregion ItemIndex property

    #region EditableObject property

    private IEditableObject EditableObject
    {
      get
      {
        return ItemsSourceHelper.GetEditableObject( this.DataContext );
      }
    }

    #endregion EditableObject property

    #region TitleBarContent Private Property

    internal static readonly DependencyProperty TitleBarContentProperty = DependencyProperty.Register(
      "TitleBarContent",
      typeof( object ),
      typeof( DataRow ) );

    #endregion TitleBarContent Private Property

    protected override void SetDataContext( object item )
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

      // When TableflowViewItemsHost is used and an EmptyDataItem is detected
      // We also want to avoid getting an animation when the container is sticky
      if( !( item is EmptyDataItem )
          && ( dataGridContext != null )
          && ( dataGridContext.DataGridControl.GetView() is TableflowView )
          && !( TableflowViewItemsHost.GetIsSticky( this ) )
          && TableflowViewItemsHost.GetShouldDelayDataContext( this ) )
      {
        this.ApplyAnimationClock( Row.CellContentOpacityProperty, null );
        this.SetValue( Row.CellContentOpacityProperty, 0d );

        // We set the DataContext to an EmptyDataItem to have the 
        // real data item set later on with an opacity animation.
        this.DataContext = this.EmptyDataItem;
        this.UpdateUnboundDataItemContext();

        Debug.Assert( m_affectDataContextOperation == null );

        // Dispatch a call to SetRealDataItem that will update
        // the DataContext of the Row and every CreatedCells
        // to the DataItem the Row must display. We use a Background priority
        // to limit the impact on scrolling.
        m_affectDataContextOperation = this.Dispatcher.BeginInvoke(
          new Action<object>( this.SetDataContextDispatched ), DispatcherPriority.Background, item );
      }
      else
      {
        this.DataContext = item;
        this.UpdateUnboundDataItemContext();

        this.ApplyAnimationClock( Row.CellContentOpacityProperty, null );
        this.ClearValue( Row.CellContentOpacityProperty );
      }
    }

    protected override void BeginEditCore()
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

      DataGridCollectionViewBase dataGridCollectionViewBase =
          ( dataGridContext == null ) ? null : dataGridContext.ItemsSourceCollection as DataGridCollectionViewBase;

      if( dataGridCollectionViewBase != null )
      {
        // We do not want to call EditItem when the item is the one in the insertionrow
        if( dataGridCollectionViewBase.CurrentAddItem != this.DataContext )
          dataGridCollectionViewBase.EditItem( this.DataContext );
      }
      else
      {
        IEditableObject editableObject = this.EditableObject;

        // editableObject can be equal to this when the datarow is directly inserted as Item in the DataGridControl.
        if( ( editableObject != null ) && ( editableObject != this ) )
          editableObject.BeginEdit();
      }

      base.BeginEditCore();
    }

    protected override void EndEditCore()
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

      base.EndEditCore();

      DataGridCollectionViewBase dataGridCollectionViewBase =
          ( dataGridContext == null ) ? null : dataGridContext.ItemsSourceCollection as DataGridCollectionViewBase;

      try
      {
        if( dataGridCollectionViewBase != null )
        {
          if( dataGridCollectionViewBase.CurrentEditItem == this.DataContext )
            dataGridCollectionViewBase.CommitEdit();
        }
        else
        {
          IEditableObject editableObject = this.EditableObject;

          // editableObject can be equal to this when the datarow is directly inserted as Items in the DataGridControl.
          if( ( editableObject != null ) && ( editableObject != this ) )
            editableObject.EndEdit();
        }
      }
      catch( Exception exception )
      {
        // Note that we do not update the created cell's Content from the source in case the IEditableObject EndEdit implementation
        // throwed an exception.  This is mainly due because we want to make sure that we do not lose all of the edited cells values.
        // This way, the end user will have the chance to correct the mistakes without loosing everything he typed.

        // If EndEdit throwed, call BeginEdit on the IEditableObject to make sure that it stays in edit mode.
        // We don't have to do this when bound to a DataGridCollectionView since it will take care of it.
        if( dataGridCollectionViewBase == null )
        {
          IEditableObject editableObject = this.EditableObject;

          // editableObject can be equal to this when the datarow is directly inserted as Items in the DataGridControl.
          if( ( editableObject != null ) && ( editableObject != this ) )
            editableObject.BeginEdit();
        }

        // This method will set a validation error on the row and throw back a DataGridValidationException so that 
        // the row stays in edition.
        Row.SetRowValidationErrorOnException( this, exception );
      }

      // Update the created cell's Content from the source in case the IEditableObject EndEdit implementation rectified
      // some values.
      this.UpdateCellsContentBindingTarget();
    }

    internal override void PreEditEnded()
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

      if( dataGridContext != null )
      {
        if( !this.RepositionPending )
        {
          using( dataGridContext.InhibitQueueBringIntoView() )
          {
            if( this.ResortPending )
              this.EnsureResort( dataGridContext );

            if( this.RegroupPending )
              this.EnsureRegroup( dataGridContext );
          }
        }
        else
        {
          this.EnsurePosition( dataGridContext );
        }
      }

      base.PreEditEnded();
    }

    protected override void CancelEditCore()
    {
      DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

      DataGridCollectionViewBase dataGridCollectionViewBase =
          ( dataGridContext == null ) ? null : dataGridContext.ItemsSourceCollection as DataGridCollectionViewBase;

      if( dataGridCollectionViewBase != null )
      {
        // We do not want to call EditItem when the item is the one in the insertionrow
        if( dataGridCollectionViewBase.CurrentEditItem == this.DataContext )
          dataGridCollectionViewBase.CancelEdit();
      }
      else
      {
        IEditableObject editableObject = this.EditableObject;

        // editableObject can be equal to this when the datarow is directly inserted as Items in the DataGridControl.
        if( ( editableObject != null ) && ( editableObject != this ) )
          editableObject.CancelEdit();
      }

      base.CancelEditCore();
    }

    internal override void PreEditCanceled()
    {
      this.ResortPending = false;
      this.RegroupPending = false;

      base.PreEditCanceled();
    }

    protected internal override void PrepareContainer( DataGridContext dataGridContext, object item )
    {
      base.PrepareContainer( dataGridContext, item );

      if( dataGridContext != null )
      {
        ( ( INotifyPropertyChanged )dataGridContext.Columns ).PropertyChanged += new PropertyChangedEventHandler( this.Columns_PropertyChanged );
        this.SetTitleBarContentBinding( dataGridContext );
        m_storedDataGridContext = dataGridContext;
      }
    }

    protected internal override void ClearContainer()
    {
      if( m_affectDataContextOperation != null )
      {
        m_affectDataContextOperation.Abort();
        m_affectDataContextOperation = null;
      }

      // Ensure to stop the opacity animation if it is currently active
      if( ( m_opacityAnimationClock != null )
          && ( m_opacityAnimationClock.CurrentState != ClockState.Stopped ) )
      {
        m_opacityAnimationClock.Completed -= this.OpacityAnimationClock_Completed;
        m_opacityAnimationClock.Controller.Stop();
        m_opacityAnimationClock = null;
      }

      base.ClearContainer();

      if( m_storedDataGridContext != null )
      {
        ( ( INotifyPropertyChanged )m_storedDataGridContext.Columns ).PropertyChanged -= new PropertyChangedEventHandler( Columns_PropertyChanged );
        m_storedDataGridContext = null;
      }
    }

    protected override Cell CreateCell( ColumnBase column )
    {
      if( column == null )
        throw new DataGridInternalException();

      return new DataCell();
    }

    protected override bool IsValidCellType( Cell cell )
    {
      return ( cell is DataCell );
    }

    internal static IDisposable DeferCollectionViewRefresh( DataGridContext dataGridContext )
    {
      Debug.Assert( dataGridContext != null );

      if( dataGridContext != null )
        return dataGridContext.Items.DeferRefresh();

      return null;
    }

    private void EnsureResort( DataGridContext dataGridContext )
    {
      if( dataGridContext == null )
        throw new ArgumentNullException( "dataGridContext" );

      if( this.IsBeingEdited )
      {
        this.ResortPending = true;
        return;
      }

      this.ResortPending = false;
      dataGridContext.EnsureResort();
    }

    private void EnsureRegroup( DataGridContext dataGridContext )
    {
      if( dataGridContext == null )
        throw new ArgumentNullException( "dataGridContext" );

      if( this.IsBeingEdited )
      {
        this.RegroupPending = true;
        return;
      }

      this.RegroupPending = false;
      dataGridContext.EnsureRegroup();
    }

    private void EnsurePosition( DataGridContext dataGridContext )
    {
      // In the case the ItemsSource is a DataGridCollectionView
      // we notify the item that correspond to the DataRow for the possible modification
      // of his data, and should then check if is position is still the same.
      if( dataGridContext == null )
        throw new ArgumentNullException( "dataGridContext" );

      if( this.IsBeingEdited )
      {
        this.RepositionPending = true;
        return;
      }

      this.RepositionPending = false;

      DataGridCollectionViewBase dataGridCollectionViewBase =
        dataGridContext.ItemsSourceCollection as DataGridCollectionViewBase;

      if( dataGridCollectionViewBase != null )
      {
        int globalSortedIndex = dataGridCollectionViewBase.IndexOf( this.DataContext );

        if( globalSortedIndex == -1 )
          return;

        dataGridCollectionViewBase.EnsurePosition( globalSortedIndex );
      }
    }

    internal void EnsurePosition( DataGridContext dataGridContext, Cell changedCell )
    {
      if( dataGridContext == null )
        throw new ArgumentNullException( "dataGridContext" );

      DataGridCollectionViewBase dataGridCollectionViewBase =
        dataGridContext.ItemsSourceCollection as DataGridCollectionViewBase;

      if( dataGridCollectionViewBase != null )
      {
        this.EnsurePosition( dataGridContext );
      }
      else
      {
        string fieldName = changedCell.FieldName;

        foreach( GroupLevelDescription groupInfo in dataGridContext.GroupLevelDescriptions )
        {
          if( groupInfo.FieldName == fieldName )
          {
            this.EnsureRegroup( dataGridContext );
            break;
          }
        }

        SortDescriptionCollection sortDescriptions = dataGridContext.Items.SortDescriptions;

        foreach( SortDescription sortDescription in sortDescriptions )
        {
          if( sortDescription.PropertyName == fieldName )
          {
            this.EnsureResort( dataGridContext );
            break;
          }
        }
      }
    }

    private void Columns_PropertyChanged( object sender, PropertyChangedEventArgs e )
    {
      if( e.PropertyName == "MainColumn" )
      {
        DataGridContext dataGridContext = DataGridControl.GetDataGridContext( this );

        this.SetTitleBarContentBinding( dataGridContext );
      }
    }

    [EditorBrowsable( EditorBrowsableState.Never )]
    protected virtual void SetTitleBarContentBinding( DataGridContext dataGridContext )
    {
      if( dataGridContext != null )
      {
        Xceed.Wpf.DataGrid.Views.ViewBase view = dataGridContext.DataGridControl.GetView();

        if( ( view is TableView )
          || ( view is TableflowView ) )
        {
          return;
        }

        Column headerColumn = dataGridContext.Columns.MainColumn as Column;

        if( headerColumn != null )
        {
          // Disable warning for DisplayMemberBinding when internaly used
#pragma warning disable 618

          BindingBase displayMemberBinding = headerColumn.DisplayMemberBinding;

#pragma warning restore 618

          if( displayMemberBinding == null )
          {
            if( dataGridContext.ItemsSourceFieldDescriptors == null )
              throw new InvalidOperationException( "An attempt was made to create a DisplayMemberBinding before the DataGridContext has been initialized." );

            string name = headerColumn.FieldName;
            ItemsSourceHelper.FieldDescriptor fieldDescriptor;
            dataGridContext.ItemsSourceFieldDescriptors.TryGetValue( name, out fieldDescriptor );

            displayMemberBinding = ItemsSourceHelper.CreateDefaultBinding(
              this.DataContext is DataRow, name, fieldDescriptor,
              false, true, typeof( object ) );

          }

          if( displayMemberBinding == null )
          {
            Debug.Assert( false, "displayMemberBinding is null." );
            this.ClearValue( DataRow.TitleBarContentProperty );
          }
          else
          {
            this.SetBinding( DataRow.TitleBarContentProperty, displayMemberBinding );
          }
        }
      }
    }

    protected internal override void PrepareDefaultStyleKey( Views.ViewBase view )
    {
      object currentThemeKey = view.GetDefaultStyleKey( typeof( DataRow ) );

      if( currentThemeKey.Equals( this.DefaultStyleKey ) == false )
      {
        this.DefaultStyleKey = currentThemeKey;
      }
    }

    #region Delayed DataContext affectation

    private DoubleAnimation m_fadeInAnimation = new DoubleAnimation();

    private void SetDataContextDispatched( object dataItem )
    {
      try
      {
        if( this.DataContext is EmptyDataItem )
        {
          this.DataContext = dataItem;
          this.UpdateUnboundDataItemContext();

          this.SetCellDataContextAnimated( false );
        }
      }
      finally
      {
        m_affectDataContextOperation = null;
      }
    }

    private void OpacityAnimationClock_Completed( object sender, EventArgs e )
    {
      m_opacityAnimationClock.Completed -= this.OpacityAnimationClock_Completed;
      m_opacityAnimationClock = null;
    }

    private void SetCellDataContextAnimated( bool hideContent )
    {
      object dataContext = this.DataContext;
      UnboundDataItem unboundDataItemContext = this.UnboundDataItemContext;
      bool isNewContext;

      foreach( Cell cell in this.CreatedCells )
      {
        Cell.AssignDataContext( cell, dataContext, unboundDataItemContext, cell.ParentColumn, out isNewContext );

        // We must refresh the Displayed template
        // since ShouldDisplayEditor always return
        // false when an EmptyDataItem is detected
        cell.RefreshDisplayedTemplate();
      }

      if( hideContent )
      {
        this.ApplyAnimationClock( Row.CellContentOpacityProperty, null );
        this.SetValue( Row.CellContentOpacityProperty, 0d );
      }
      else 
      {
        m_fadeInAnimation.From = 0d;
        m_fadeInAnimation.To = 1d;
        m_fadeInAnimation.Duration = TimeSpan.FromMilliseconds( TableflowView.GetRowFadeInAnimationDuration( DataGridControl.GetDataGridContext( this ) ) );

        if( m_opacityAnimationClock != null )
        {
          m_opacityAnimationClock.Controller.Pause();
          m_opacityAnimationClock.Completed -= this.OpacityAnimationClock_Completed;
          m_opacityAnimationClock = null;
        }

        m_opacityAnimationClock = ( AnimationClock )m_fadeInAnimation.CreateClock( true );
        m_opacityAnimationClock.Completed += this.OpacityAnimationClock_Completed;

        this.ApplyAnimationClock( Row.CellContentOpacityProperty, m_opacityAnimationClock );
        m_opacityAnimationClock.Controller.Begin();
      }
    }

    private AnimationClock m_opacityAnimationClock; // = null;

    #endregion

    #region PRIVATE PROPERTIES

    private bool ResortPending
    {
      get
      {
        return m_flags[ ( int )DataRowFlags.ResortPending ];
      }
      set
      {
        m_flags[ ( int )DataRowFlags.ResortPending ] = value;
      }
    }

    private bool RegroupPending
    {
      get
      {
        return m_flags[ ( int )DataRowFlags.RegroupPending ];
      }
      set
      {
        m_flags[ ( int )DataRowFlags.RegroupPending ] = value;
      }
    }

    private bool RepositionPending
    {
      get
      {
        return m_flags[ ( int )DataRowFlags.RepositionPending ];
      }
      set
      {
        m_flags[ ( int )DataRowFlags.RepositionPending ] = value;
      }
    }

    #endregion PRIVATE PROPERTIES

    #region PRIVATE FIELDS

    private BitVector32 m_flags = new BitVector32();

    private DataGridContext m_storedDataGridContext; // = null
    private DispatcherOperation m_affectDataContextOperation; // = null;

    #endregion PRIVATE FIELDS

    #region Flags Enum

    [Flags]
    private enum DataRowFlags
    {
      ResortPending = 1,
      RegroupPending = 2,
      RepositionPending = 4,
    }

    #endregion Flags Enum
  }
}
