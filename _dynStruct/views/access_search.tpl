% include header
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    table = $('#access').DataTable( {
      "processing": true,
      "serverSide": true,
      "ajax": {
        "url": "access_get?id_block={{id_block}}&id_member={{id_member}}"
      },
      "createdRow": function( row, data, dataIndex ) {
        if ( data[0] == "<code>write</code>" ) {
	$(row).css('font-weight', 'bold');
  }
  $('td:eq(3)', row).css('font-weight', 'normal');
	},
  % if defined('in_block_view'):
  "columnDefs": [
  { "visible": false, "targets": 4 }
    ]  
  % end
  }
  )

  table.columns().every( function () {
    var that = this;
    $( 'input', this.footer() ).on( 'keyup change', function () {
      if ( that.search() !== this.value ) {
        that
            .search( this.value )
            .draw();}})
    $( 'select', this.footer() ).on( 'keyup change', function () {
      if ( that.search() !== this.value ) {
        that
            .search( this.value )
            .draw();}})})
  });
</script>

% if not defined('in_page'):
<div class="container-fluid">
  <h1 class="text-center"> Access search</h1>
% end
  <table id="access" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> access </th>
	<th> offset </th>
	<th> size (in bytes)</th>
	<th> agent </th>
	<th> block_id </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> <select class="form-control"><option value=""></option><option value="write">write</option><option value="read">read</option></select></th>
	<th> <input class="form-control" type="text" placeholder="Search offset" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search agent" /> </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
% if not defined('in_page'):
</div>
% end
% include footer
