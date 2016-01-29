% include header

<div class="container-fluid">
  <h1 class="text-center"> Block informations</h1>
  <h3 class="text-center"> <span class="text-primary">{{'0x%x' % (block["start"] & 0xffffffffffffffff)}} - {{'0x%x' % (block["end"] & 0xffffffffffffffff)}}</span> ({{'%d' % (block["end"] - block["start"])}} bytes) </h3>

  <table class="table table-bordered">
    <tr>
      <td> </td>
      <td> through </td>
      <td> caller</td>
    </tr>
    <tr class="success">
      <td class="text-center"><code>alloc</code></td>
      <td> <code>{{"malloc" if not block["alloc_by_realloc"] else "realloc"}}</code></td>
      <td> <code><span class="text-danger">{{'0x%x' % (block["alloc_pc"] & 0xffffffffffffffff)}}</span>:<span class="{{'text-success' if block["alloc_sym"] else 'text-danger'}}"><strong>{{block["alloc_sym"] if block["alloc_sym"] else '0x%x' % (block["alloc_func"] & 0xffffffffffffffff)}}</strong></span>+{{'0x%x' % ((block["alloc_pc"] - block["alloc_func"]) & 0xffffffffffffffff)}}@<span class="text-warning">{{block["alloc_module"]}}</span></code></td>
    </tr>
% if block["free"]:
    <tr class="danger">
      <td class="text-center"><code>free</code></td>
      <td> <code>{{"free" if not block["free_by_realloc"] else "realloc"}} </code></td>
      <td> <code><span class="text-danger">{{'0x%x' % (block["free_pc"] & 0xffffffffffffffff)}}</span>:<span class="{{'text-success' if block["free_sym"] else 'text-danger'}}"><strong>{{block["free_sym"] if block["free_sym"] else '0x%x' % (block["free_func"] & 0xffffffffffffffff)}}</strong></span>+{{'0x%x' % ((block["free_pc"] - block["free_func"]) & 0xffffffffffffffff)}}@<span class="text-warning">{{block["free_module"]}}</span></code></td>
    </tr>
% end
  </table>
  
  todo: link to struct view + link to remove from the struct or add to a struct
  <h1 class="text-center"> Block access</h1>
% include('access_search.tpl', in_page=True, in_block_view=True, id_block=block["id_block"], id_struct=None)
</div>

% include footer
