% include header

<div class="container-fluid">
  <h1 class="text-center"> {{struct.name}} informations</h1>
  <h3 class="text-center"> Size : {{struct.size}} bytes</h3>
  todo: change block association (add, remove) structure editing.
  <h1 class="text-center"> Structure members</h1>
% include('member_search.tpl', in_page=True, id_struct=struct.id)
  <h1 class="text-center"> Instances</h1>
% include('block_search.tpl', in_page=True, in_struct_view=True, id_struct=struct.id)
</div>

% include footer
