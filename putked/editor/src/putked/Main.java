package putked;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import putked.Interop.MemInstance;
import javafx.application.Application;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.collections.transformation.FilteredList;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.FileChooser;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.FileChooser.ExtensionFilter;
import javafx.util.Callback;

class OpenEditors
{
	public Editor _editor;
	public MemInstance _mi;
	public Tab _tab;
	public String _contentHash;
}

public class Main extends Application 
{
	public static Main s_instance;
		
	private TabPane m_pane;
	public ObjectLibrary m_objectLibrary;
	private static ArrayList<Editor> m_editors = new ArrayList<>();
	private static ArrayList<DataImporter> m_importers = new ArrayList<DataImporter>();
	private static ArrayList<OpenEditors> m_openEditors = new ArrayList<OpenEditors>();
	private static Editor m_defaultEditor =  new PropertyEditor();
	private static ArrayList<EditorPluginDescription> m_pluginDescs = new ArrayList<EditorPluginDescription>();
	private static String s_builderPath;
	private Builds m_builds;
	
    public static void main(String[] args) 
    {
    	if (args.length > 0 && args[0].equals("--clear")) {
    		try {
	        	Preferences prefs = Preferences.userRoot().node("putked");
	        	prefs.remove("config");
	        	prefs.flush();
    		} catch (BackingStoreException e) {
    			
    		}
    	}
    	if (args.length > 1 && args[0].equals("--path")) {
    		try {
	        	Preferences prefs = Preferences.userRoot().node("putked");
	        	prefs.put("config", args[1]);
	        	prefs.flush();
    		} catch (BackingStoreException e) {
    			
    		}
    	}
    	initEditor();
        Application.launch(args);
    }
    
    public static void addPluginDesc(EditorPluginDescription plug)
    {
    	m_pluginDescs.add(plug);
    }
    
    public static void initEditor()
    {
    	addEditor(new PropertyEditor()); 	
    }
    
    public static void addImporter(DataImporter importer)
    {
    	m_importers.add(importer);
    }
    
    public static void addEditor(Editor e)
    {
    	m_editors.add(e);
    }
    
    public static ArrayList<DataImporter> getImporters()
    {
    	return m_importers;
    }
    
    public static ArrayList<Editor> getEditors()
    {
    	return m_editors;
    }
    
    public void startEditing(String path)
    {   	
    	if (path.length() == 0)
    		return;
    	
    	int prio = 0;
    	Editor e = m_defaultEditor;

    	Interop.MemInstance mi = Interop.s_wrap.load(path);
    	if (mi == null)
    		return;
    	
    	Interop.Type t = mi.getType();

    	// pick lowest priority editor
    	for (Editor tmp : m_editors) {
			int p = tmp.getPriority();
			if (p < prio && tmp.canEdit(t)) {
				prio = p;
				e = tmp;
    		}
    	}
    	
    	startEditing(path, e);
    }
    
    public void startEditing(String path, Editor editor)
    {
    	Interop.MemInstance mi = Interop.s_wrap.load(path);
    	if (mi == null)
    		return;
    	if (!editor.canEdit(mi.getType()))
    		return;
    	
    	for (int i=0;i<m_openEditors.size();i++)
    	{
    		OpenEditors oe = m_openEditors.get(i);
    		if (oe._mi.getPath().equals(mi.getPath())) // && oe._editor == editor)
    		{
    		   	m_pane.getSelectionModel().select(oe._tab);
    		   	return;
    		}
    	}
   
    	OpenEditors oe = new OpenEditors();
    	
    	Tab t = addTab(mi, editor.createUI(mi), path);
    	t.setOnCloseRequest( (evt) -> {
    		m_openEditors.remove(oe);    		
    	});
    	
    	oe._editor = editor;
    	oe._mi = mi;
    	oe._tab = t;
    	oe._contentHash = mi.getContentHash();
    	m_openEditors.add(oe);
    }
    
    public Stage makeDialogStage(javafx.scene.Scene scene)
    {
        scene.getStylesheets().add(getClass().getResource("application.css").toExternalForm());
    	Stage s = new Stage();
//    	s.getIcons().add(new Image(getClass().getResourceAsStream("misc/putkico-5.png")));
    	s.initModality(Modality.APPLICATION_MODAL);
    	s.setTitle("Question");
    	s.setScene(scene);
    	return s;    	
    }
    
    public String[] askForResources()
    {
    	return null;
    }
    
    public String askForInstancePath(Interop.Type p)
    {
    	ObservableList<String> all = FXCollections.observableArrayList();
    	ObservableList<ObjectLibrary.ObjEntry> objs = m_objectLibrary.getAllObjects();
    	for (ObjectLibrary.ObjEntry o : objs)
    	{
    		if (o.type != null && o.type.hasParent(p))
    			all.add(o.path);
    	}
    	
       	ListView<String> opts = new ListView<>();
    	opts.setItems(all);
    	opts.getSelectionModel().select(0);

    	VBox box = new VBox();
    	Button ok = new Button("OK");
    	Button cancel = new Button("Cancel");
    	ok.setMaxWidth(Double.MAX_VALUE);
    	cancel.setMaxWidth(Double.MAX_VALUE);
    	
    	class Tmp {
    		String out = null;
    	};
    	
    	final Tmp holder = new Tmp();

    	Scene scene = new Scene(box, 400, 300);
        Stage stage = makeDialogStage(scene);
   	
    	ok.setOnAction((evt) -> {
    		holder.out = opts.getSelectionModel().getSelectedItem();
    		stage.hide();
    	});

    	cancel.setOnAction( (evt) -> {
    		stage.hide();
    	});

    	box.getChildren().setAll(opts, ok, cancel);    	
    	stage.showAndWait();
		return holder.out;
    }
    
    
    class TypeOption
    {
    	public String name;
    	Interop.Type type;
    };

    public Interop.Type askForType()
    {
    	return askForSubType(null, false);
    }

    public Interop.Type askForSubType(Interop.Type p, boolean asAux)
    {
    	ArrayList<Interop.Type> all = Interop.s_wrap.getAllTypes();
    	ObservableList<TypeOption> out = FXCollections.observableArrayList();
    	for (Interop.Type t : all)
    	{
    		if (p == null || t.hasParent(p) && (!asAux || t.permitAsAuxInstance()))
    		{
    			TypeOption to = new TypeOption();
    			to.name = "[" + t.getModule() + "] - " + t.getName();
    			to.type = t;
    			out.add(to);
    		}
    	}
    	
    	if (out.size() == 1)
    		return out.get(0).type;
    	
    	FilteredList<TypeOption> filter = new FilteredList<TypeOption>(out, (obj) -> true);
    	ListView<TypeOption> opts = new ListView<>();
    	opts.setItems(filter);
    	opts.getSelectionModel().select(0);

    	VBox box = new VBox();
    	Button ok = new Button("OK");
    	Button cancel = new Button("Cancel");
    	ok.setMaxWidth(Double.MAX_VALUE);
    	cancel.setMaxWidth(Double.MAX_VALUE);
    	
    	TextField searchField = new TextField();

    	searchField.textProperty().addListener( (obs, oldval, newval) -> {
    		filter.setPredicate( (to) -> {
    			return to.name.contains(newval);
    		});
    	});

    	searchField.setText("");
    	
        opts.setCellFactory(new Callback<ListView<TypeOption>, ListCell<TypeOption>>(){
            @Override
            public ListCell<TypeOption> call(ListView<TypeOption> p) {
               ListCell<TypeOption> cell = new ListCell<TypeOption>(){
                    @Override
                    protected void updateItem(TypeOption t, boolean bln) {
                        super.updateItem(t, bln);
                        if (t != null) {
                            setText(t.name);
                        } else {
                        	setText("");
                        }
                    }
                };
                return cell;
            }
        });
        
    	class Tmp {
    		Interop.Type out = null;
    	};
    	
    	final Tmp holder = new Tmp();

    	Scene scene = new Scene(box, 300, 400);
    	Stage stage = makeDialogStage(scene);
   	
    	ok.setOnAction((evt) -> {
    		holder.out = opts.getSelectionModel().getSelectedItem().type;
    		stage.hide();
    	});

    	cancel.setOnAction( (evt) -> {
    		stage.hide();
    	});

    	box.getChildren().setAll(searchField, opts, ok, cancel);    	
    	stage.showAndWait();
 	
		return holder.out;
    }
    
    public static class ImportFinalizationQuestion
    {
    	public String proposedPath;
    	public String proposedResPath;
    	public boolean accepted;
    }
    
    public void askImportFinalization(ImportFinalizationQuestion question, Node aux)
    {    	
    	VBox contents = new VBox();
    	contents.getStyleClass().add("vbox-dialog");
    	contents.getStyleClass().add("root");
    	
    	TextField propPath = new TextField();
    	if (question.proposedPath != null)
    	{
        	Label L = new Label("Object path");
        	propPath.setText(question.proposedPath);
    		contents.getChildren().add(L);
    		contents.getChildren().add(propPath);
    	}
    	
    	TextField propResPath = new TextField();
    	if (question.proposedResPath != null)
    	{
        	Label L = new Label("Resource path");
        	propResPath.setText(question.proposedResPath);
        	contents.getChildren().add(L);
        	contents.getChildren().add(propResPath);
    	}
    	
    	if (aux != null)
    		contents.getChildren().add(aux);

    	contents.setMaxHeight(Double.MAX_VALUE);

    	VBox box = new VBox();
    	Button ok = new Button("OK");
    	Button cancel = new Button("Cancel");
    	ok.setMaxWidth(Double.MAX_VALUE);
    	cancel.setMaxWidth(Double.MAX_VALUE);
    	
    	HBox btns = new HBox();
    	btns.getChildren().setAll(cancel, ok);
    	btns.setAlignment(Pos.BOTTOM_CENTER);
    	
    	box.getChildren().add(contents);
    	box.getChildren().add(btns);

    	Scene scene = new Scene(box, 300, 400);
    	Stage stage = makeDialogStage(scene);
   	
    	ok.setOnAction((evt) -> {
    		question.accepted = true;
    		question.proposedResPath = propResPath.getText();
    		question.proposedPath = propPath.getText();
    		stage.hide();
    	});

    	cancel.setOnAction( (evt) -> {
    		question.accepted = false;
    		stage.hide();
    	});

    	stage.showAndWait();
    }    
    
    public Tab addTab(MemInstance mi, Node n, String title)
    {
    	Tab t = new Tab(title);
    	t.setContent(n);
    	m_pane.getTabs().add(t);
    	m_pane.getSelectionModel().select(t);
    	return t;
    }
 
    public static void interopInit(String interopPath, String dllPath, String builderPath)
    {
    	System.out.println("Interop init " + interopPath + " and " + dllPath); 
    	if (Interop.Load(interopPath))
    	{
    		Interop.Initialize(dllPath, m_projectPath);
    	}
    	s_builderPath = builderPath;
    }
    
    public static String getBuilderPath()
    {
    	return s_builderPath;
    }
    
    private static String m_projectPath = null;
    private static ConfigParser m_confParser = null;
    
    public static String getProjectPath()
    {
    	return m_projectPath;
    }
    
    @Override
    public void start(Stage stage) throws BackingStoreException, IOException
    {
    	s_instance = this;
    	
    	Preferences prefs = Preferences.userRoot().node("putked");
    	
    	String configPath = prefs.get("config", null);
    	File configFile = null;
    	if (configPath != null) {
    		configFile = new File(configPath);
    		if (!configFile.exists())
    			configFile = null;
    	}
       	
    	if (configFile == null)
    	{
    		FileChooser fileChooser = new FileChooser();
    		fileChooser.setTitle("Open root");
    		fileChooser.getExtensionFilters().addAll(new ExtensionFilter("PutkEd Configuration Files", "putked.config"));
    		configFile = fileChooser.showOpenDialog(null);
       	}
    	
    	if (configFile == null) {
    		prefs.clear();
    		prefs.flush(); 
    		System.exit(1);
    		return;
    	}
     	
    	m_projectPath = configFile.getParent();
    	m_confParser = new ConfigParser(configFile);
    	
    	for (String val : m_confParser.getMulti("plugin")) {
    		try {
	    		@SuppressWarnings("unchecked")
				Class<EditorPluginDescription> desc = (Class<EditorPluginDescription>) Main.class.getClassLoader().loadClass(val);
	    		EditorPluginDescription epd = desc.newInstance(); 
	    		System.out.println("Plugin (" + epd.getName() + ":" + epd.getVersion() + ") loaded.");
	    		m_pluginDescs.add(epd);
	        } catch (ClassNotFoundException e) { 		
	    		System.out.println("Error loading plugin [" + val + "] " + e.toString());
	        } catch (IllegalAccessException e) { 		
	    		System.out.println("Error loading plugin [" + val + "] " + e.toString());
	        } catch (InstantiationException e) { 		
	        	System.out.println("Error loading plugin [" + val + "]" + e.toString());
	        }
    	}
    	
       	EditorPluginDescription buildLoader = null;
    	for (EditorPluginDescription desc : m_pluginDescs) {
    		switch (desc.getType()) {
    			case PLUGIN_PROJECT_BUILD:
    				buildLoader = desc;
    				break;
    			case PLUGIN_PROJECT_DEV_BUILD:
    				if (buildLoader == null) {
    					buildLoader = desc;
    				}
    				break;
    			default:
   					break;
    		}
    	}
    	
    	System.out.println("Using build loader:" + buildLoader.getName());
    	buildLoader.start();
    	System.out.println("Did build setup");
    	
    	// survived this far so save it.
    	prefs.put("config",  configFile.getAbsolutePath());
    	prefs.flush();
    	    	
    	for (EditorPluginDescription desc : m_pluginDescs) {
    		switch (desc.getType()) {
    			case PLUGIN_EDITOR:
    				System.out.println("Booting editor plugin " + desc.getName());
    				desc.start();
    			default:
   					break;
    		}
    	}    	
    	
    	stage.setTitle(m_confParser.getSingle("title"));
    	
    	m_pane = new TabPane();
    	m_objectLibrary = new ObjectLibrary();
    	    	
    	SplitPane pane = new SplitPane();    	
    	pane.orientationProperty().set(Orientation.VERTICAL);
    	pane.getItems().add(m_pane);
    	pane.getItems().add(m_objectLibrary.getRoot());
    

    	
    	m_builds = new Builds();
    	Node buildBar = m_builds.createBuildBarUI();
    	
    	VBox outer = new VBox();
    	outer.getChildren().setAll(pane, buildBar);
    	
    	VBox.setVgrow(pane,  Priority.ALWAYS);
    	VBox.setVgrow(buildBar, Priority.NEVER);
    	
        final Scene scene = new Scene(outer, 800, 800);
        scene.getStylesheets().add(getClass().getResource("application.css").toExternalForm());       
        stage.setScene(scene);
        
//    	stage.getIcons().add(new Image(getClass().getResourceAsStream("misc/putkico-5.png")));
        stage.show();
    }     
}
