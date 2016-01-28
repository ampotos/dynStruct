% include header

<div class="container">
  <h1 class="text-center"> Block informations</h1>
  <h3 class="text-center"> <span class="text-primary">{{hex(block["start"])}} - {{hex(block["end"])}}</span> ({{hex(block["end"] - block["start"])}} bytes) </h3>

  <table class="table table-bordered">
    <tr>
      <td> </td>
      <td> through </td>
      <td> called at</td>
      <td> caller</td>
    </tr>
    <tr class="success">
      <td class="text-center">alloc</td>
      <td> {{"malloc" if not block["alloc_by_realloc"] else "realloc"}} </td>
      <td> <span class="text-danger">{{hex(block["alloc_pc"])}}</span>:<span class="{{'text-success' if block["alloc_sym"] else 'text-danger'}}">{{block["alloc_sym"] if block["alloc_sym"] else hex(block["alloc_func"])}}</span>+{{hex(block["alloc_pc"] - block["alloc_func"])}} </td>
      <td> <span class="text-danger">{{hex(block["alloc_func"])}}</span>@<span class="text-warning">{{block["alloc_module"]}}</span> </td>
    </tr>
% if block["free"]:
    <tr class="danger">
      <td class="text-center">free</td>
      <td> {{"free" if not block["free_by_realloc"] else "realloc"}} </td>
      <td> <span class="text-danger">{{hex(block["free_pc"])}}</span>:<span class="{{'text-success' if block["free_sym"] else 'text-danger'}}">{{block["free_sym"] if block["free_sym"] else hex(block["free_func"])}}</span>+{{hex(block["free_pc"] - block["free_func"])}} </td>
      <td> <span class="text-danger">{{hex(block["free_func"])}}</span>@<span class="text-warning">{{block["free_module"]}}</span> </td>
    </tr>
% end
  </table>

  todo: link to struct view + link to remove from the struct or add to a struct
  <h1 class="text-center"> Block access</h1>
  <div class="embed-responsive embed-responsive-4by3">
    <iframe class="embed-responsive-item" id="search_frame" onload="javascript:hide_navbar()" width="100%" src="/access_search?id_block={{block["id_block"]}}" scrolling="no""></iframe>
  </div>

</div>

% include footer
