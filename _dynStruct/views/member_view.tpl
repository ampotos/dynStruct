% include header
<div class="container-fluid">
  <h1 class="text-center"> {{name_member}} informations</h1>

% if member.is_array:
  <table class="table table-bordered">
    <tr>
      <td> number of units </td>
      <td> size of unit (in bytes)</td>
      <td> unit type </td>
      <td> total size (in bytes) </td>
    </tr>
    <tr>
      <td> {{member.number_unit}} </th>
      <td> {{member.size_unit}} </td>
      <td> <span class="text-warning">{{member.t}}</span></td>
      <td> {{member.size}} </td>
    </tr>
  </table>
  
% elif member.is_struct:
todo edit struct  
  <h1 class="text-center"> Structure members</h1>
% include('member_search.tpl', in_page=True, id_struct=id_member)

% elif member.is_array_struct:
  todo edit struct
  <table class="table table-bordered">
    <tr>
      <td> number of units </td>
      <td> size of unit (in bytes)</td>
      <td> unit type </td>
      <td> total size (in bytes) </td>
    </tr>
    <tr>
      <td> {{member.number_unit}} </th>
      <td> {{member.size_unit}} </td>
      <td> <span class="text-warning">{{member.sub_struct.name}}</span></td>
      <td> {{member.size}} </td>
    </tr>
  </table>

  <h1 class="text-center"> {{member.sub_struct.name}} members</h1>
% include('member_search.tpl', in_page=True, id_struct=id_member)
  
% else:
  
  <table class="table table-bordered">
    <tr>
      <td> size (in bytes)</td>
      <td> type </td>
    </tr>
    <tr>
      <td> {{member.size}} </td>
      <td> <span class="text-warning">{{member.t}}</span></td>
    </tr>
  </table>
  
% end

  <h1 class="text-center"> Member access</h1>
% include('access_search.tpl', in_page=True, id_block=None, id_member=id_member)

</div>

% include footer
