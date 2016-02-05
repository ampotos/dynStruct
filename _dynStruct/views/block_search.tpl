% include header
<script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    table = $('#blocks').DataTable( {
      "processing": true,
      "serverSide": true,
      "ajax": {
        "url": "block_get?id_struct={{id_struct}}"
      },
  % if defined('in_struct_view'):
  "columnDefs": [
  { "visible": false, "targets": 4 }
    ]
  % end
  })

  table.columns().every( function () {
    var that = this;
    $( 'input', this.footer() ).on( 'keyup change', function () {
      if ( that.search() !== this.value ) {
        that
            .search( this.value )
            .draw();}})})
  });
</script>

% if not defined('in_page'):
<div class="container-fluid">
  <h1 class="text-center"> Block search</h1>
% end
  <table id="blocks" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> start </th>
	<th> size </th>
	<th> malloc caller </th>
	<th> free caller </th>
	<th> struct </th>
	<th> detailed view </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> <input class="form-control" type="text" placeholder="Search adress" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search malloc caller" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search free caller" /> </th>
	<th>  </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
% if not defined('in_page'):
</div>
% end
% include footer
