% include header

<div class="container-fluid">
  <h1 class="text-center"> {{struct.name}} informations</h1>
% if not edit:
  <h3 class="text-center"> Size : {{struct.size}} bytes</h3>
  <h3 class="text-center"> <a href="/struct_edit?id={{struct.id}}"> Edit structure </a> </h3>
% else:
  <h3 class="text-center"> <a href="/struct_remove?id={{struct.id}}"> Remove structure </a> </h3>
  <form action="/struct_do_edit?id={{struct.id}}" method="post" class="form-inline text-center">
    <input type="text" name="name" value="{{struct.name}}" class="form-control" />
    <input type="submit" value="Edit" class="btn btn-primary" />
  </form>
% end
  <h1 class="text-center"> Structure members</h1>
% include('member_search.tpl', in_page=True, id_struct=struct.id)
  <h1 class="text-center"> Instances</h1>
% include('block_search.tpl', in_page=True, in_struct_view=True, id_struct=struct.id)
</div>

% include footer
