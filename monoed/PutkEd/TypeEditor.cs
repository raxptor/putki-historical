using System;
using System.Collections.Generic;
using Gtk;

namespace PutkEd
{
	public interface TypeEditor
	{
		Widget GetRoot();

		void SetObject(DLLLoader.MemInstance mi, DLLLoader.PutkiField fi, int araryIndex);

		List<AssetEditor.RowNode> GetChildRows();
		void OnConnect(AssetEditor root);
	}
}

