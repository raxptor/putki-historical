package putked;

import javafx.geometry.Insets;
import javafx.scene.*;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

public class PropertyEditor implements Editor
{
	public PropertyEditor()
	{

	}
	
	public int getPriority()
	{
		return 0;
	}
	
	public String getName()
	{
		return "Property Editor";
	}
	
	public boolean canEdit(Interop.Type tp)
	{
		return true;
	}
	
	public Node createUI(Interop.MemInstance object)
	{
		StructEditor se = new StructEditor(object, null, false);
		
		VBox k = new VBox(se.createUI());
		k.setPadding(new Insets(5));
		
		HBox desc = new HBox();
		Label path = new Label("Editing [" + object.getPath() + "] (" + object.getType().getModule() + " - " + object.getType().getName() + ")");
		Button save = new Button("Save");
		save.setOnAction( (actionEvent) -> {
			object.diskSave();
			actionEvent.consume();
		});
		
		desc.getChildren().setAll(path, save);
		desc.getStyleClass().add("proped-header");
		
		ScrollPane sp = new ScrollPane(k);
		sp.setPrefHeight(-1);
		VBox.setVgrow(sp,  Priority.ALWAYS);
		VBox.setVgrow(desc, Priority.NEVER);
			
		VBox box = new VBox();
		box.getChildren().setAll(desc, sp);
		return box;
	}
}

