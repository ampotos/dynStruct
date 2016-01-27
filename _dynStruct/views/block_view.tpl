% include header

<div class="container">
  <h1 class="text-center"> Block informations</h1>
  <p class="text-center"> {{hex(block["start"])}} - {{hex(block["end"])}} </p>
  <p class="text-center"> allocation done at {{hex(block["alloc_pc"])}} 
  % if block["alloc_sym"]:
  ({{block["free_sym"]}} + {{hex(block["alloc_pc"] - block["alloc_func"])}}) by {{"malloc" if not block["alloc_by_realloc"] else "realloc"}}</p>  
  <p class="text-center">function "{{block["alloc_sym"]}}" starting at {{hex(block["alloc_func"])}} in module "{{block["alloc_module"]}}" </p>
  % else:
  by {{"malloc" if not block["alloc_by_realloc"] else "realloc"}} in function starting at {{hex(block["alloc_func"])}} in module "{{block["alloc_module"]}}" </p>
  % end

  % if block["free"]:
  <p class="text-center"> free done at {{hex(block["free_pc"])}} 
  % if block["free_sym"]:
  ({{block["free_sym"]}} + {{hex(block["free_pc"] - block["free_func"])}}) by {{"free" if not block["free_by_realloc"] else "realloc"}}</p>
  <p class="text-center">function "{{block["free_sym"]}}" starting at {{hex(block["free_func"])}} in module "{{block["free_module"]}}" </p>
  % else:
  by {{"free" if not block["free_by_realloc"] else "realloc"}} in function starting at {{hex(block["free_func"])}} in module "{{block["free_module"]}}" </p>
  % end
    
  % else:
  <p class="text-center"> not free </p>
  % end
    
  % if block["struct"]:
  <p class="text-center"> The block is associate with the structure named {{block["struct"].name}} (todo link to the struct and link to remove this block from the struct) </p>
  % else:
  <p class="text-center"> No structure associate with this block (todo create new struct link here) </p>
  % end
  
  <h1 class="text-center"> Block access</h1>
  <div class="embed-responsive embed-responsive-4by3">
    <iframe width="100%" src="/access_search?id_block={{block["id_block"]}}&iframe=1"></iframe>
  </div>
</div>

% include footer
