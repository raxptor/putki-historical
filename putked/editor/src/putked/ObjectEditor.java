package putked;

import java.util.ArrayList;

import putked.Interop.*;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.geometry.VPos;
import javafx.scene.*;
import javafx.scene.control.*;
import javafx.scene.input.DragEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.TransferMode;
import javafx.scene.layout.*;

class StringEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public StringEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }

    @Override
    public Node createUI()
    {
        m_f.setArrayIndex(m_index);
        TextField tf = new TextField(m_f.getString(m_mi));
        tf.textProperty().addListener( (obs, oldValue, newValue) -> {
            m_f.setArrayIndex(m_index);
            m_f.setString(m_mi, newValue);
        });
        return tf;
    }
}

class FileEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public FileEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }

    @Override
    public Node createUI()
    {
        TextField tf = new TextField();
        tf.getStyleClass().add("file-field");
        tf.textProperty().addListener( (obs, oldValue, newValue) -> {
    		tf.getStyleClass().remove("error");
        	java.io.File f = new java.io.File(Interop.translateResPath(newValue));
        	if (!f.exists() || f.isDirectory())
        		tf.getStyleClass().add("error");
        	m_f.setArrayIndex(m_index);
        	m_f.setString(m_mi,  newValue);
        });
        tf.setText(m_f.getString(m_mi));
        return tf;        
    }
}

class BooleanEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public BooleanEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }

    @Override
    public Node createUI()
    {
        m_f.setArrayIndex(m_index);
        CheckBox cb = new CheckBox(m_f.getName());
        cb.setSelected(m_f.getInteger(m_mi) != 0);
        cb.selectedProperty().addListener( (obs, old, ny) -> {
            m_f.setArrayIndex(m_index);
            m_f.setInteger(m_mi, ny ? 1 : 0);
        });
        return cb;
    }
}


class EnumEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public EnumEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }

    @Override
    public Node createUI()
    {
        m_f.setArrayIndex(m_index);
        ComboBox<String> cb = new ComboBox<>();
        ArrayList<String> values = new ArrayList<>();

        int i=0;
        while (true)
        {
            String s = m_f.getEnumPossibility(i);
            if (s == null)
                break;
            values.add(s);
            i++;
        }

        cb.getItems().setAll(values);
        cb.setValue(m_f.getEnum(m_mi));
        cb.valueProperty().addListener( (obs, oldValue, newValue) -> {
            m_f.setArrayIndex(m_index);
            m_f.setEnum(m_mi,  newValue);
        });

        return cb;
    }
}

class IntegerEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;
    long m_min, m_max;

    public IntegerEditor(MemInstance mi, Field f, int index, long min, long max)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
        m_min = min;
        m_max = max;
    }

    @Override
    public Node createUI()
    {
        m_f.setArrayIndex(m_index);
        TextField tf = new TextField(new Long(m_f.getInteger(m_mi)).toString());
        tf.getStyleClass().add("integer-field");
        tf.textProperty().addListener( (obs, oldValue, newValue) -> {
            try
            {
                long val = Long.parseLong(newValue);
                if (val < m_min || val > m_max)
                	throw new NumberFormatException("Out of range");
                m_f.setArrayIndex(m_index);
                m_f.setInteger(m_mi, val);
                tf.getStyleClass().remove("error");
            }
            catch (NumberFormatException u)
            {
                tf.getStyleClass().remove("error");
                tf.getStyleClass().add("error");
            }
        });
        return tf;
    }
}

class FloatEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public FloatEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }

    @Override
    public Node createUI()
    {
        m_f.setArrayIndex(m_index);
        TextField tf = new TextField(new Float(m_f.getFloat(m_mi)).toString());
        tf.getStyleClass().add("float-field");
        tf.textProperty().addListener( (obs, oldValue, newValue) -> {
            try
            {
                float f = Float.parseFloat(newValue);
                m_f.setArrayIndex(m_index);
                m_f.setFloat(m_mi, f);
                tf.getStyleClass().remove("error");
            }
            catch (NumberFormatException u)
            {
                tf.getStyleClass().add("error");
            }
        });
        return tf;
    }
}

class PointerEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    int m_index;

    public PointerEditor(MemInstance mi, Field f, int index)
    {
        m_mi = mi;
        m_f = f;
        m_index = index;
    }


    @Override
    public Node createUI()
    {
        VBox tot = new VBox();

        HBox ptrbar = new HBox();
        m_f.setArrayIndex(m_index);

        ptrbar.setMaxWidth(Double.MAX_VALUE);

        TextField tf = new TextField(m_f.getPointer(m_mi));
        tf.setEditable(false);
        tf.setDisable(true);
        tf.setMinWidth(200);
        HBox.setHgrow(tf, Priority.ALWAYS);

        Button clear = new Button("X");
        Button point = new Button("*");

        clear.getStyleClass().add("rm-button");
        point.getStyleClass().add("point-button");

        ptrbar.getChildren().setAll(tf, point, clear);
        tot.getChildren().setAll(ptrbar);

        clear.setOnAction( (evt) -> {
                m_f.setArrayIndex(m_index);
                m_f.setPointer(m_mi, "");
                tf.textProperty().set("");
                tot.getChildren().setAll(ptrbar);
                ptrbar.getChildren().setAll(tf, point);
        });

        point.setOnAction( (evt) -> {

            if (m_f.isAuxPtr())
            {
                Interop.Type t = Main.s_instance.askForSubType(Interop.s_wrap.getTypeByName(m_f.getRefType()), true);
                if (t != null)
                {
                    MemInstance naux = m_mi.createAuxInstance(t);

                    m_f.setArrayIndex(m_index);
                    m_f.setPointer(m_mi, naux.getPath());
                    tf.textProperty().set(EditorCreatorUtil.makeInlineAuxTitle(naux));

                    VBox aux = makeObjNode(naux);
                    aux.setVisible(true);
                    aux.setManaged(true);

                    Button expand = new Button("V");
                    expand.setOnAction(new EventHandler<ActionEvent>() {
                        @Override
                        public void handle(ActionEvent event) {
                            aux.setVisible(!aux.isVisible());
                            aux.setManaged(aux.isVisible());
                        }
                    });

                    // configure for this
                    ptrbar.getChildren().setAll(expand, tf, point, clear);
                    tot.getChildren().setAll(ptrbar, aux);
                }
            }
            else
            {
                String path = Main.s_instance.askForInstancePath(Interop.s_wrap.getTypeByName(m_f.getRefType()));
                if (path != null)
                {
                    m_f.setArrayIndex(m_index);
                    m_f.setPointer(m_mi, path);
                    tf.textProperty().set(path);
                    ptrbar.getChildren().setAll(tf, point, clear);
                    tot.getChildren().setAll(ptrbar);
                }
            }
        });
        
        Interop.Type refType = Interop.s_wrap.getTypeByName(m_f.getRefType());
        
        if (!m_f.isAuxPtr())
        {
        	tot.setOnDragOver(new EventHandler<DragEvent>() {
        	    public void handle(DragEvent event) {
	                if (event.getGestureSource() != tf && event.getDragboard().hasString()) {
	                	MemInstance dragging = Interop.s_wrap.load(event.getDragboard().getString());
	                	if (dragging != null && dragging.getType().hasParent(refType)) {
	                		event.acceptTransferModes(TransferMode.LINK);
	                	}
	                }
        	        event.consume();
        	    }
        	});       	
        	
	        tot.setOnDragEntered(new EventHandler<DragEvent>() {
	            public void handle(DragEvent event) {
	            	tf.getStyleClass().remove("drag-drop-ok");
	            	tf.getStyleClass().remove("drag-drop-not-ok");
	                if (event.getGestureSource() != tf && event.getDragboard().hasString()) {
	                	MemInstance dragging = Interop.s_wrap.load(event.getDragboard().getString());
	                	if (dragging != null && dragging.getType().hasParent(refType)) {              	
	                		tf.getStyleClass().add("drag-drop-ok");
	                	} else {
	                		tf.getStyleClass().add("drag-drop-not-ok");       		
	                	}
	                }
	                event.consume();
	            }
	        });
	        
	        tot.setOnDragDropped(new EventHandler<DragEvent>() {
	            public void handle(DragEvent event) {
	            	boolean success = false;
	                if (event.getGestureSource() != tf && event.getDragboard().hasString()) {
	                	String path = event.getDragboard().getString();
	                	MemInstance dragging = Interop.s_wrap.load(path);
	                	if (dragging != null && dragging.getType().hasParent(refType)) {
	                        m_f.setArrayIndex(m_index);
	                        m_f.setPointer(m_mi, path);
	                        tf.textProperty().set(path);
	                        ptrbar.getChildren().setAll(tf, point, clear);
	                        tot.getChildren().setAll(ptrbar); 
	                        success = true;
	                	}
	                }
	                event.setDropCompleted(success);
	                event.consume();
	             }
	        });
	        
	        tot.setOnDragExited(new EventHandler<DragEvent>() {
	            public void handle(DragEvent event) {
	            	tf.getStyleClass().remove("drag-drop-ok");
	            	tf.getStyleClass().remove("drag-drop-not-ok");
	            	event.consume();
	            }
	        }); 
        }
        
        if (m_f.isAuxPtr())
        {
            m_f.setArrayIndex(m_index);
            String ref = m_f.getPointer(m_mi);
            if (ref.length() > 0)
            {
                MemInstance mi = Interop.s_wrap.load(ref);
                if (mi != null)
                {
                    VBox aux = makeObjNode(mi);

                    Button expand = new Button("V");
                    expand.setOnAction(new EventHandler<ActionEvent>() {
                        @Override
                        public void handle(ActionEvent event) {
                            aux.setVisible(!aux.isVisible());
                            aux.setManaged(aux.isVisible());
                        }
                    });

                    // configure for this
                    ptrbar.getChildren().setAll(expand, tf, point, clear);
                    tot.getChildren().setAll(ptrbar, aux);
                    tf.getStyleClass().remove("error");
                    tf.textProperty().set(EditorCreatorUtil.makeInlineAuxTitle(mi));
                }
                else
                {
                    tf.getStyleClass().add("error");
                }
            }
        }
        
        if (!m_f.isAuxPtr())
        {
			ptrbar.setOnMousePressed(new EventHandler<MouseEvent>() {
				@Override
				public void handle(MouseEvent event) {
					if (event.isPrimaryButtonDown() && event.getClickCount() == 2) {
						Main.s_instance.startEditing(tf.getText());
					}
				}
			});
        }

        tot.setFillWidth(true);
        tot.setMaxWidth(Double.MAX_VALUE);
        return tot;
    }

    private VBox makeObjNode(MemInstance mi)
    {
        VBox aux = new VBox();
        StructEditor se = new StructEditor(mi, "AUX", false);
        ArrayList<Node> tmp = new ArrayList<>();
        tmp.add(se.createUI());
        aux.setFillWidth(true);
        aux.getChildren().setAll(tmp);
        aux.setPadding(new Insets(4));
        aux.setStyle("-fx-border-insets: 1;");
        aux.setVisible(false);
        aux.setManaged(false);
        return aux;
    }
}

class ArrayEditor implements FieldEditor
{
    MemInstance m_mi;
    Field m_f;
    ArrayList<Node> m_editors;
    VBox m_box;

    public ArrayEditor(MemInstance mi, Field field)
    {
        m_mi = mi;
        m_f = field;
    }

    @Override
    public Node createUI()
    {
        m_box = new VBox();
        rebuild();
        return m_box;
    }

    private void rebuild()
    {
        Label hl = new Label(m_f.getName() + ": Array of " + m_f.getArraySize(m_mi) + " items(s)");
        hl.setMaxWidth(Double.MAX_VALUE);
        hl.setAlignment(Pos.BASELINE_CENTER);
        hl.setPrefHeight(30);

        Button add = new Button("+" + m_f.getName());

        add.setOnAction( (evt) -> {
            int newIndex = m_f.getArraySize(m_mi);
            m_f.setArrayIndex(newIndex);
            m_f.arrayInsert(m_mi);
            rebuild();
        });

        m_box.getChildren().setAll(hl, buildGridPane(), add);
        m_box.setFillWidth(true);
    }

    private Button mkRemoveBtn(int idx)
    {
        Button rm = new Button("-");
        rm.setOnAction((v) -> {
            m_f.setArrayIndex(idx);
            System.out.println("Erasing at " + idx + " with array size " + m_f.getArraySize(m_mi));
            m_f.arrayErase(m_mi);
            rebuild();
        });
        return rm;
    }

    private GridPane buildGridPane()
    {
        m_editors = new ArrayList<>();
        int size = m_f.getArraySize(m_mi);
        for (int i=0;i<size;i++)
        {
            FieldEditor fe = ObjectEditor.createEditor(m_mi, m_f, i, false);
            m_editors.add(fe.createUI());
        }

        GridPane gridpane = new GridPane();
        gridpane.setMaxWidth(Double.MAX_VALUE);

        ColumnConstraints column0 = new ColumnConstraints(-1,-1,Double.MAX_VALUE);
        ColumnConstraints column1 = new ColumnConstraints(-1,-1,Double.MAX_VALUE);

        if (((m_f.getType() == 3 && m_f.isAuxPtr()) || m_f.getType() == 5))
            column1.setHgrow(Priority.ALWAYS);

        gridpane.getColumnConstraints().setAll(column0, column1);
        for (int i=0;i<m_editors.size();i++)
        {
            Label lbl = new Label(" " + i);
            lbl.setMaxHeight(Double.MAX_VALUE);
            lbl.setAlignment(Pos.CENTER);
            lbl.getStyleClass().add("array-index");
            if ((i&1) == 1)
                lbl.getStyleClass().add("odd");

            gridpane.add(lbl,  0,  i);
            GridPane.setValignment(lbl, VPos.TOP);
            gridpane.add(m_editors.get(i),  1,  i);

            Button rm_btn = mkRemoveBtn(i);
            rm_btn.getStyleClass().add("rm-button");
            gridpane.add(rm_btn, 2, i);
            GridPane.setValignment(rm_btn,  VPos.TOP);
        }

        return gridpane;
    }
}

class StructEditor implements FieldEditor
{
    MemInstance m_mi;
    String m_name;
    boolean m_inline;

    public StructEditor(MemInstance mi, String name, boolean inline)
    {
        m_mi = mi;
        m_name = name;
        m_inline = inline;
    }

    @Override
    public Node createUI()
    {
        ArrayList<Node> nodes = new ArrayList<>();

        boolean giveRect = false;

        Label header = null;

        if (m_name != null && !m_name.equals("parent"))
        {
            header = new Label(m_name + " (" + m_mi.getType().getName() + ")");
            header.setMaxWidth(Double.MAX_VALUE);
            header.getStyleClass().add("struct-header");
            header.setAlignment(Pos.CENTER);
            header.setMaxHeight(Double.MAX_VALUE);

            if (!m_inline)
            {
                nodes.add(header);
            }

            giveRect = true;
        }

        for (int i=0;true;i++)
        {
            Interop.Field f = m_mi.getType().getField(i);
            if (f == null)
                break;
            if (!f.showInEditor())
            	continue;
            
            FieldEditor fe = ObjectEditor.createEditor(m_mi, f, 0, f.isArray());
            Node ed = fe.createUI();

            // array or struct or pointer or bools dont get labels.
            if (f.isArray() || f.getType() == Interop.FT_STRUCT_INSTANCE || f.getType() == Interop.FT_BOOL)
            {
                nodes.add(ed);
            }
            else
            {
                if (f.getType() == Interop.FT_POINTER && f.isAuxPtr())
                {
                    // aux objs get vbox
                    VBox b = new VBox();
                    b.setMaxWidth(Double.MAX_VALUE);
                    b.setFillWidth(true);
                    b.getChildren().setAll(EditorCreatorUtil.makeLabel(f, 0), ed);
                    nodes.add(b);
                }
                else
                {
                    if (m_inline)
                    {
                        Label l = new Label(f.getName());
                        l.getStyleClass().add("inline-label");
                        ed.getStyleClass().add("inline-value");
                        nodes.add(l);
                        nodes.add(ed);
                    }
                    else
                    {
                        // fields get hbox
                        HBox hb = new HBox();
                        hb.setMaxWidth(Double.MAX_VALUE);
                        hb.getChildren().setAll(EditorCreatorUtil.makeLabel(f, 0), ed);
                        HBox.setHgrow(ed, Priority.ALWAYS);
                        nodes.add(hb);
                    }
                }
            }
        }

        if (m_inline)
        {
            HBox box = new HBox();
            box.getChildren().setAll(nodes);
            if (header == null)
                return box;

            HBox main = new HBox();
            main.getChildren().setAll(EditorCreatorUtil.makeInlineEditorHeader(m_name), box);
            return main;
        }

        VBox box = new VBox();
        box.setFillWidth(true);
        box.getChildren().setAll(nodes);

        if (giveRect)
        {
            box.setMaxWidth(Double.MAX_VALUE);
        }
        return box;
    }
}

public class ObjectEditor
{
    VBox m_props;
    MemInstance m_mi;
    StructEditor m_root;
	private static ArrayList<FieldEditorCreator> s_cedts = new ArrayList<>();

    public ObjectEditor(MemInstance mi)
    {
        m_props = new VBox();
        m_mi = mi;

        m_root = new StructEditor(mi, null, false);
        m_props.getChildren().setAll(m_root.createUI());
        m_props.setMinWidth(400);
    }

    public void constructField(Interop.Field f)
    {

    }

    public Parent getRoot()
    {
        return m_props;
    }

	public static void addCustomTypeEditorCreator(FieldEditorCreator c)
	{
		s_cedts.add(c);
	}
	
    public static FieldEditor createEditor(MemInstance mi, Field field, int index, boolean asArray)
    {
    	for (FieldEditorCreator c : s_cedts) {
    		FieldEditor res = c.createEditor(mi, field, index,  asArray);
    		if (res != null)
    			return res;
    	}
    	
        field.setArrayIndex(index);
        
        if (asArray)
            return new ArrayEditor(mi, field);
        
    	for (FieldEditorCreator c : s_cedts) {
    		FieldEditor res = c.createEditor(mi, field, index, false);
    		if (res != null)
    			return res;
    	}

        switch (field.getType())
        {
            case Interop.FT_STRUCT_INSTANCE:
            {
                String name = field.getName();
                if (field.isArray())
                    name += "[" + index + "]";

                MemInstance _mi = field.getStructInstance(mi);
                System.out.println("field ed [" + _mi.getType().getInlineEditor() + "]");
                if (_mi.getType().getInlineEditor().equals("Vec4"))
                    return new StructEditor(_mi, name, true);

                return new StructEditor(_mi, name, false);
            }
            case Interop.FT_POINTER:
                return new PointerEditor(mi, field, index);
            case Interop.FT_INT32:
                return new IntegerEditor(mi, field, index, Integer.MIN_VALUE, Integer.MAX_VALUE);
            case Interop.FT_UINT32:
                return new IntegerEditor(mi, field, index, 0, 0xffffffffL );
            case Interop.FT_BYTE:
                return new IntegerEditor(mi, field, index, 0, 255);
            case Interop.FT_BOOL:
                return new BooleanEditor(mi, field, index);
            case Interop.FT_FILE:
                return new FileEditor(mi, field, index);
            case Interop.FT_FLOAT:
                return new FloatEditor(mi, field, index);
            case Interop.FT_ENUM:
                return new EnumEditor(mi, field, index);
            default:
                return new StringEditor(mi, field, index);
        }
    }	
}
