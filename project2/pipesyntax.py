import json 

def parse_plan(plan):
  node_type = plan.get("Node Type", "Unknown")
  children = plan.get("Plans", [])
  startup_cost = plan.get("Startup Cost", "N/A")
  total_cost = plan.get("Total Cost", "N/A")
  lines = []
  
  # skip Hash node 
  if node_type == "Hash":
    return ""
  
  for child in children:
    child_result = parse_plan(child)
    if child_result:
      lines.append(child_result)
  
  cost_diaply = f"{total_cost}"
  
  # handle current node
  if node_type == "Seq Scan":
    # so far havent done other cases
    table = plan.get("Relation Name", plan.get("Alias", "Unknown"))
    # show startup and total cost
    line = f"FROM {table} -- Cost: {startup_cost} → {total_cost}"
    if "Filter" in plan:
      filter_expr = (
        plan["Filter"]
        .replace("::date", "")
        .replace("::timestamp without time zone", "")
        .replace("::numeric", "")
      )
      line += f"\n|> WHERE {filter_expr} -- Cost: {total_cost}"
    lines.append(line)
  elif node_type == "Aggregate":
    mode = plan.get("Partial Mode", "")
    
    if "Output" in plan and isinstance(plan["Output"], list) and plan["Output"]:
        output_expr = ", ".join(plan["Output"])
    else:
      output_expr = "aggregate values"
    
    if mode == "Partial":
      lines.append(f"|> AGGREGATE PARTIAL {output_expr} -- Cost: {cost_diaply}")
    elif mode == "Finalize":
      lines.append(f"|> AGGREGATE FINALIZE {output_expr} -- Cost: {cost_diaply}")
    else:
      lines.append(f"|> AGGREGATE {output_expr} -- Cost: {cost_diaply}")
  elif node_type == "Gather":
    lines.append(f"|> GATHER -- Cost: {cost_diaply}")
  elif node_type == "Limit":
    limit_rows = plan.get("Plan Rows", 1)
  elif node_type == "Sort":
    keys = plan.get("Sort Key", [])
    lines.append(f"|> ORDER BY {', '.join(keys)} -- Cost: {cost_diaply}")
  elif node_type == "Nested Loop":
    lines.append(f"|> NESTED LOOP -- Cost: {cost_diaply}")
  elif node_type == "Merge Join":
    cond = plan.get("Merge Cond", "")
    if cond:
      lines.append(f"|> MERGE JOIN ON {cond} -- Cost: {cost_diaply}")
    else:
      lines.append(f"|> MERGE JOIN -- Cost: {cost_diaply}")
  elif node_type == "Hash Join":
    cond - plan.get("Hash Cond", "")
    if cond:
      lines.append(f"|> HASH JOIN ON {cond} -- Cost: {cost_diaply}")
    else:
      lines.append(f"|> HASH JOIN -- Cost: {cost_diaply}")
  return "\n".join(lines)

  
def convert_qep_to_pipe_syntax(json_str):
  try:
    tree_json = json.loads(json_str)
    plan_root = tree_json[0]["Plan"]
    return parse_plan(plan_root)

  except json.JSONDecodeError:
    print("error process qep tree json")
    return None
  
if __name__ == "__main__":
  with open('/Users/zuozhiyi/Desktop/sc3020/raw.json', 'r') as f:
    json_str = f.read()
  print(convert_qep_to_pipe_syntax(json_str))