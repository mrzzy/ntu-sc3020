import tkinter as tk
from tkinter import ttk, messagebox
from typing import Dict, Any
import psycopg2 
def todo(function):
    """Not implemented yet."""
    name = function.__name__
    print(f"{name} is not implemented yet.")

class GUI:
    """GUI for Project2"""
    
    def __init__(self):
        self.root = None
        self.query_text = None
        self.result_text = None
        self.qep_text = None
        self.connect = None
        self.cur = None
        
    def run(self):
        """Initialize and run the GUI, but no function yet."""
        self.root = tk.Tk()
        self.root.title("SQL QEP to Pipe Syntax Converter")
        self.root.geometry("1200x800")
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky="nsew")
        #Connection block
        conn_frame = ttk.LabelFrame(main_frame, text="Database Connection", padding="5")
        conn_frame.grid(row=0, column=0, columnspan=2, sticky="we", pady=5)
        
        #Connection fields with default inputs
        ttk.Label(conn_frame, text="Host:").grid(row=0, column=0, padx=5)
        self.host_entry = ttk.Entry(conn_frame)
        self.host_entry.grid(row=0, column=1, padx=5)
        self.host_entry.insert(0, "localhost")
        #The things waste my storage
        ttk.Label(conn_frame, text="Database:").grid(row=0, column=2, padx=5)
        self.db_entry = ttk.Entry(conn_frame)
        self.db_entry.grid(row=0, column=3, padx=5)
        self.db_entry.insert(0, "postgres")
        #just default
        ttk.Label(conn_frame, text="User:").grid(row=0, column=4, padx=5)
        self.user_entry = ttk.Entry(conn_frame)
        self.user_entry.grid(row=0, column=5, padx=5)
        self.user_entry.insert(0, "postgres")
        #just input pw, but not work for now
        ttk.Label(conn_frame, text="Password:").grid(row=0, column=6, padx=5)
        self.pwd_entry = ttk.Entry(conn_frame, show="*")
        self.pwd_entry.grid(row=0, column=7, padx=5)
        self.pwd_entry.insert(0, "SC3020")
        #default port, change it ur own
        ttk.Label(conn_frame, text="Port:").grid(row=0, column=8, padx=5)
        self.port_entry = ttk.Entry(conn_frame, width=6)
        self.port_entry.grid(row=0, column=9, padx=5)
        self.port_entry.insert(0, "5432")
        
        #button
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.connect_db)
        self.connect_btn.grid(row=0, column=10, padx=10)
        
        # Query input block
        query_frame = ttk.LabelFrame(main_frame, text="SQL Query", padding="5")
        query_frame.grid(row=1, column=0, sticky=("wens"), pady=5)
        
        self.query_text = tk.Text(query_frame, height=10, width=70)
        self.query_text.grid(row=0, column=0, sticky=("wens"))
        query_scroll = ttk.Scrollbar(query_frame, orient=tk.VERTICAL, command=self.query_text.yview)
        query_scroll.grid(row=0, column=1, sticky=("ns"))
        self.query_text['yscrollcommand'] = query_scroll.set
        
        #sample, but maybe also the test query??
        sample_query = "SELECT c.c_custkey, c.c_name, c.c_nationkey, n.n_name\nFROM customer c\nJOIN nation n ON c.c_nationkey = n.n_nationkey\nWHERE c.c_acctbal > 1000\nORDER BY c.c_custkey\nLIMIT 10;"
        self.query_text.insert("1.0", sample_query)
        
        # Convert button
        self.convert_btn = ttk.Button(main_frame, text="Convert", command=self._mock_convert_query)
        self.convert_btn.grid(row=2, column=0, pady=5)
        
        # output frame
        results_frame = ttk.Frame(main_frame)
        results_frame.grid(row=3, column=0, columnspan=2, sticky=("wens"))
        
        # QEP display
        qep_frame = ttk.LabelFrame(results_frame, text="Query Execution Plan", padding="5")
        qep_frame.grid(row=0, column=0, sticky=("wens"), padx=5)
        
        self.qep_text = tk.Text(qep_frame, height=15, width=70)
        self.qep_text.grid(row=0, column=0, sticky=("wens"))
        qep_scroll = ttk.Scrollbar(qep_frame, orient=tk.VERTICAL, command=self.qep_text.yview)
        qep_scroll.grid(row=0, column=1, sticky=("ns"))
        self.qep_text['yscrollcommand'] = qep_scroll.set
        
        # Pipe display
        pipe_frame = ttk.LabelFrame(results_frame, text="Pipe Syntax", padding="5")
        pipe_frame.grid(row=0, column=1, sticky=("wens"), padx=5)
        
        self.result_text = tk.Text(pipe_frame, height=15, width=70)
        self.result_text.grid(row=0, column=0, sticky=("wens"))
        result_scroll = ttk.Scrollbar(pipe_frame, orient=tk.VERTICAL, command=self.result_text.yview)
        result_scroll.grid(row=0, column=1, sticky=("ns"))
        self.result_text['yscrollcommand'] = result_scroll.set
        
        #weight
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(3, weight=1)
        results_frame.columnconfigure((0, 1), weight=1)
        results_frame.rowconfigure(0, weight=1)
        def on_closing():
            if self.connect:
              if self.cur:
                  self.cur.close()
              self.connect.close()
            if self.root:
                self.root.destroy()
            
        self.root.protocol("WM_DELETE_WINDOW", on_closing)
        # Start GUI
        self.root.mainloop()
        
    def connect_db(self):
        """Connect to database"""
        try:
          para = {
              "host": self.host_entry.get(),
              "database": self.db_entry.get(),
              "user": self.user_entry.get(),
              "password": self.pwd_entry.get(),
              "port": self.port_entry.get()
          }
          if self.connect:
              if self.cur:
                  self.cur.close()
              self.connect.close()
          print("connecting")
          self.connect = psycopg2.connect(**para)
          self.cur = self.connect.cursor()
          #try 
          print("connected")
          self.cur.execute("SELECT version();")
          version = self.cur.fetchone()
          messagebox.showinfo("Success", f"Connected to PostgreSQL\n{version[0]}")
          
        except psycopg2.Error as e:
          messagebox.showerror("Error", f"Failed to connect to PostgreSQL\n{str(e)}")
          

    def _mock_connect_db(self):
        """fake info"""
        messagebox.showinfo("Success", "Connected to database successfully!")

    @todo
    def convert_query(self):
        """Convert SQL query to Pipe Syntax"""
        pass
    def _mock_convert_query(self):
        """fake output, not sure if the result is correct or not. """
        query = self.query_text.get("1.0", tk.END).strip()
        if not query:
            messagebox.showwarning("Warning", "Please enter a SQL query")
            return
        mock_qep = self._generate_mock_qep(query)
        self.qep_text.delete("1.0", tk.END)
        self.qep_text.insert("1.0", mock_qep)

        mock_pipe_syntax = self._generate_mock_pipe_syntax()
        self.result_text.delete("1.0", tk.END)
        for component, cost in mock_pipe_syntax:
            self.result_text.insert(tk.END, f"{component}  // Cost: {cost:.2f}\n")
    @todo
    def _generate_qep(self, query):
        """Generate Query Execution Plan"""
        pass
    def _generate_mock_qep(self, query):
        """fake qep"""
        return """
{
  "Plan": {
    "Node Type": "Limit",
    "Startup Cost": 1000.00,
    "Total Cost": 1010.00,
    "Plan Rows": 10,
    "Plan Width": 100,
    "Plans": [
      {
        "Node Type": "Sort",
        "Startup Cost": 900.00,
        "Total Cost": 950.00,
        "Sort Key": ["c.c_custkey"],
        "Plans": [
          {
            "Node Type": "Hash Join",
            "Join Type": "Inner",
            "Startup Cost": 500.00,
            "Total Cost": 800.00,
            "Hash Cond": "(c.c_nationkey = n.n_nationkey)",
            "Plans": [
              {
                "Node Type": "Seq Scan",
                "Relation Name": "customer c",
                "Alias": "c",
                "Startup Cost": 0.00,
                "Total Cost": 400.00,
                "Filter": "c.c_acctbal > 1000"
              },
              {
                "Node Type": "Hash",
                "Startup Cost": 100.00,
                "Total Cost": 100.00,
                "Plans": [
                  {
                    "Node Type": "Seq Scan",
                    "Relation Name": "nation n",
                    "Alias": "n",
                    "Startup Cost": 0.00,
                    "Total Cost": 100.00
                  }
                ]
              }
            ]
          }
        ]
      }
    ]
  }
}
"""
    @todo
    def _generate_pipe_syntax(self):
        """Generate Pipe Syntax from QEP"""
        pass
    
    def _generate_mock_pipe_syntax(self):
        """fake pipe syntax"""
        return [
            ("Scan(customer) | Filter(c_acctbal > 1000)", 400.00),
            ("Scan(nation)", 100.00),
            ("Hash(nation)", 100.00),
            ("HashJoin(customer.c_nationkey = nation.n_nationkey)", 800.00),
            ("Sort(c_custkey)", 950.00),
            ("Limit(10)", 1010.00)
        ]

if __name__ == "__main__":
    app = GUI()
    app.run()